#include <wayfire/plugin.hpp>
#include <wayfire/opengl.hpp>
#include <wayfire/pixman.hpp>
#include <wayfire/core.hpp>
#include <wayfire/view.hpp>
#include <wayfire/output.hpp>
#include <wayfire/render-manager.hpp>
#include <wayfire/workspace-manager.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/nonstd/reverse.hpp>
#include <wayfire/util/duration.hpp>
#include <wayfire/view-transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>

/* these includes for logging */
#include <wayfire/util/log.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>

#include "../main.hpp"

constexpr const char *pixswitcher_autobot = "pixswitcher-2d";
constexpr const char *pixswitcher_decepticon = "pixswitcher-2d-bg";
constexpr float bg_dim_factor = 0.6;
constexpr float back_scale = 0.66;

/* for testing */
constexpr int max_count = 5;

using namespace wf::animation;

class PixSwitcherPaintAttribs
{
 public:
   PixSwitcherPaintAttribs(const duration_t& duration) :
     scale_x(duration, 1, 1), scale_y(duration, 1, 1),
     /* off_x(duration, 0, 0), off_y(duration, 0, 0), */
     translation_x(duration, 0, 0), translation_y(duration, 0, 0),
     alpha(duration, 1, 1)
       {}

   timed_transition_t scale_x, scale_y;
   /* timed_transition_t off_x, off_y; */
   timed_transition_t translation_x, translation_y;
   timed_transition_t alpha;
};

enum PixSwitcherDirection
{
   PIXSWITCHER_DIRECTION_FORWARD = 0,
   PIXSWITCHER_DIRECTION_BACKWARD = 1,
};

static constexpr bool view_expired(int index)
{
   return index < 0 || index > max_count;
}

struct PixSwitcherView
{
   wayfire_view view;
   PixSwitcherPaintAttribs attribs;
   wf::geometry_t orig_geom = {0, 0, 0, 0};

   int index;
   PixSwitcherView(duration_t duration) : attribs(duration)
     {}

   void refresh_start()
     {
        for_each([] (timed_transition_t& t) { t.restart_same_end(); });
     }

   void to_end()
     {
        for_each([] (timed_transition_t& t) { t.set(t.end, t.end); });
     }

 private:
   void for_each(std::function<void(timed_transition_t& t)> call)
     {
        /* call(attribs.off_x); */
        /* call(attribs.off_y); */
        call(attribs.translation_x);
        call(attribs.translation_y);
        call(attribs.scale_x);
        call(attribs.scale_y);
        call(attribs.alpha);
     }
};

class PixSwitcher : public wf::plugin_interface_t
{
   wf::option_wrapper_t<double> thumb_scale{"pixswitcher/thumbnail_scale"};
   wf::option_wrapper_t<int> speed{"pixswitcher/bg_dim_speed"};

   duration_t duration{speed};
   duration_t bg_dim_duration{speed};
   timed_transition_t bg_dim{bg_dim_duration};

   std::vector<PixSwitcherView> views;

   uint32_t modifiers = 0;
   bool active = false;
   wf::geometry_t ps_box;
   uint32_t count = 0;

 public:

   void init() override
     {
        grab_interface->name = "pixswitcher";
        grab_interface->capabilities = wf::CAPABILITY_MANAGE_COMPOSITOR;

        auto og = output->get_relative_geometry();
        float bw = og.width * 0.5;
        float bh = og.height * 0.5;
        float x = og.x + ((og.width - bw) / 2);
        float y = og.y + ((og.height - bh) / 2);

        ps_box.x = x;
        ps_box.y = y;
        ps_box.width = bw;
        ps_box.height = bh;

        output->add_key(wf::option_wrapper_t<wf::keybinding_t>
                        {"pixswitcher/next_view"}, &next_binding);
        output->add_key(wf::option_wrapper_t<wf::keybinding_t>
                        {"pixswitcher/prev_view"}, &prev_binding);
        output->connect_signal("view-detached", &view_removed);

        grab_interface->callbacks.keyboard.mod = [=] (uint32_t mod, uint32_t state)
          {
             if ((state == WLR_KEY_RELEASED) && (mod & modifiers))
               pixswitcher_terminate();
          };

        grab_interface->callbacks.cancel = [=] () { deinit_switcher(); };
     }

   void fini() override
     {
        if (output->is_plugin_active(grab_interface->name))
          deinit_switcher();

        output->rem_binding(&next_binding);
        output->rem_binding(&prev_binding);
        output->disconnect_signal("view-detached", &view_removed);
     }

   /* keybinding callbacks for cycling between views */
   wf::key_callback next_binding = [=] (auto)
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Next binding");
        return handle_switch_request(PIXSWITCHER_DIRECTION_FORWARD);
     };

   wf::key_callback prev_binding = [=] (auto)
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Previous binding");
        return handle_switch_request(PIXSWITCHER_DIRECTION_BACKWARD);
     };

   /* output signal callback when a view gets removed */
   wf::signal_callback_t view_removed = [=] (wf::signal_data_t *data)
     {
        /* if we are not running, then nothing to do */
        if (!output->is_plugin_active(grab_interface->name))
          {
             count = 0;
             return;
          }

        bool need_action = false;
        wayfire_view view = get_signaled_view(data);

        for (auto& sv : views)
          need_action |= (sv.view == view);

        if (!need_action) return;

        if (active)
          arrange_views();
        else
          {
             cleanup_views([=] (PixSwitcherView& sv)
                           { return sv.view == view; });
          }
     };

   wf::effect_hook_t damage = [=] ()
     {
        output->render->damage_whole();
     };

   PixSwitcherView create_view(wayfire_view view)
     {
        if (!view->get_transformer(pixswitcher_autobot))
          {
             view->add_transformer(std::make_unique<wf::view_2D>(view),
                                   pixswitcher_autobot);
          }

        PixSwitcherView pv{duration};
        pv.view = view;
        pv.index = count++;
        pv.orig_geom = view->get_wm_geometry();

        return pv;
     }

   std::vector<wayfire_view> get_workspace_views() const
     {
        auto ws = output->workspace->get_current_workspace();
        auto all = output->workspace->get_views_on_workspace
          (ws, wf::WM_LAYERS | wf::LAYER_MINIMIZED);

        decltype(all) mapped_views;
        for (auto view : all)
          {
             if (view->is_mapped())
               mapped_views.push_back(view);
          }

        return mapped_views;
     }

   float get_view_normal_alpha(wayfire_view view)
     {
        if (view->minimized && (views.empty() || (view != views[0].view)))
          return 0.0;

        return 1.0;
     }

   wayfire_view get_unfocused_view()
     {
        for (auto& pv : views)
          {
             /* FIXME: check index */
             if (!view_expired(pv.index) && (pv.index != 3))
               return pv.view;
          }

        return nullptr;
     }

   void arrange_views()
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Arrange Views");

        views.clear();
        count = 0;

        duration.start();
        bg_dim.set(1, bg_dim_factor);
        bg_dim_duration.start();

        auto wviews = get_workspace_views();
        for (auto v : wviews)
          views.push_back(create_view(v));

        if (wviews.size() == 2)
          views.push_back(create_view(wviews.back()));

        /* FIXME: index ? or position (left, center, right) ? */
        /* arrange_view(views[0], views[0].index); */

        /* if (wviews.size() > 1) */
        /*   arrange_view(views.back(), views.back().index); */

        count = views.size();

        for (int i = 0; i < (int)views.size(); i++)
          arrange_view(views[i], views[i].index);
     }

   void dearrange_views()
     {
        wayfire_view fv = nullptr;

        wlr_log(WLR_DEBUG, "PixSwitcher Dearrange Views");

        if (count_active_views() == 2)
          fv = get_unfocused_view();

        for (auto& pv : views)
          {
             pv.attribs.translation_x.restart_with_end(0);
             pv.attribs.translation_y.restart_with_end(0);

             /* pv.attribs.off_x.restart_with_end(0); */
             /* pv.attribs.off_y.restart_with_end(0); */

             pv.attribs.scale_x.restart_with_end(1.0);
             pv.attribs.scale_y.restart_with_end(1.0);

             pv.attribs.alpha.restart_with_end(get_view_normal_alpha(pv.view));

             if (pv.view == fv)
               {
                  pv.attribs.alpha.end = 0.0;
                  fv = nullptr;
               }

             pv.view->move(pv.orig_geom.x, pv.orig_geom.y);
          }

        bg_dim.restart_with_end(1);
        bg_dim_duration.start();
        duration.start();
        active = false;

        if (views.size())
          output->focus_view(views[0].view, true);
     }

   std::vector<wayfire_view> get_bg_views() const
     {
        auto ws = output->workspace->get_current_workspace();
        return output->workspace->get_views_on_workspace(ws, wf::BELOW_LAYERS);
     }

   std::vector<wayfire_view> get_overlay_views() const
     {
        auto ws = output->workspace->get_current_workspace();
        return output->workspace->get_views_on_workspace(ws, wf::ABOVE_LAYERS);
     }

   bool handle_switch_request(int dir)
     {
        if (get_workspace_views().empty())
          {
             wlr_log(WLR_DEBUG, "PixSwitcher Workspace Views Empty !!");
             return false;
          }

        if (!output->is_plugin_active(grab_interface->name))
          {
             if (!init_switcher())
               return false;
          }

        if (!active)
          {
             active = true;
             auto grab = grab_interface->grab();
             assert(grab);

             focus_next(dir);
             arrange_views();
             modifiers = wf::get_core().get_keyboard_modifiers();
          }
        else
          next_view(dir);

        return true;
     }

   void pixswitcher_terminate()
     {
        cleanup_expired();
        dearrange_views();
        grab_interface->ungrab();
     }

   bool init_switcher()
     {
        if (!output->activate_plugin(grab_interface))
          return false;

        output->render->add_effect(&damage, wf::OUTPUT_EFFECT_PRE);
        output->render->set_renderer(pixswitcher_renderer);
        output->render->set_redraw_always();

        return true;
     }

   void deinit_switcher()
     {
        output->deactivate_plugin(grab_interface);

        output->render->rem_effect(&damage);
        output->render->set_renderer(nullptr);
        output->render->set_redraw_always(false);

        for (auto& view : output->workspace->get_views_in_layer(wf::ALL_LAYERS))
          {
             view->pop_transformer(pixswitcher_autobot);
             view->pop_transformer(pixswitcher_decepticon);
          }

        views.clear();
        count = 0;

        wf::stack_order_changed_signal data;
        data.output = output;
        wf::get_core().emit_signal("output-stack-order-changed", &data);
     }

   int count_active_views()
     {
        std::set<wayfire_view> aviews;
        for (auto& pv : views)
          aviews.insert(pv.view);
        return aviews.size();
     }

   float get_center_offset()
     {
        return output->get_relative_geometry().width / max_count;
     }

   void move_view(PixSwitcherView& pv, int dir)
     {
        float zs = 0;

        wlr_log(WLR_DEBUG, "PixSwitcher Move View");
        wlr_log(WLR_DEBUG, "\tIndex: %d", pv.index);

        pv.attribs.translation_x.restart_with_end
          (pv.attribs.translation_x.end + get_center_offset() * dir);
        pv.attribs.translation_y.restart_same_end();

        wlr_log(WLR_DEBUG, "\tTrans: %3.0f %3.0f",
                pv.attribs.translation_x + get_center_offset() * dir,
                pv.attribs.translation_y);
#if 0
        /* pv.attribs.off_x.restart_with_end */
        /*   (pv.attribs.off_x.end + get_center_offset() * pv.index); */
        /* pv.attribs.off_y.restart_same_end(); */

        if (pv.index == 1) // center
          zs = 1;
        else if (view_expired(pv.index + dir))
          zs = 0;
        else
          zs = -1;
#endif

        wlr_log(WLR_DEBUG, "\tNew Position: %3.0f %3.0f",
                pv.attribs.translation_x.end + get_center_offset() * pv.index,
                pv.attribs.translation_y.end);

        /* wlr_log(WLR_DEBUG, "\tNew Position: %3.0f %3.0f", */
        /*         pv.attribs.off_x.end + get_center_offset() * pv.index, */
        /*         pv.attribs.off_y.end); */

        /* pv.view->move(pv.attribs.off_x.end + get_center_offset() * pv.index, */
        /*               pv.attribs.off_y.end); */

        pv.attribs.scale_x.restart_with_end
          (pv.attribs.scale_x.end * back_scale);
        pv.attribs.scale_y.restart_with_end
          (pv.attribs.scale_y.end * back_scale);

        pv.attribs.alpha.restart_with_end(view_expired(pv.index) ? 0.3 : 1.0);
     }

   void arrange_center_view(PixSwitcherView& pv)
     {
        auto og = output->get_relative_geometry();
        auto bbox = pv.view->get_bounding_box(pixswitcher_autobot);

        wlr_log(WLR_DEBUG, "PixSwitcher Arrange Center View");
        wlr_log(WLR_DEBUG, "\tSwitcher Box: %d %d %d %d",
                ps_box.x, ps_box.y, ps_box.width, ps_box.height);
        wlr_log(WLR_DEBUG, "\tView Box: %d %d %d %d",
                bbox.x, bbox.y, bbox.width, bbox.height);

        /* float dx = ps_box.x; */
        /* float dy = ps_box.y; */
        float dx = 300;
        float dy = 300;
        wlr_log(WLR_DEBUG, "\tDx: %3.0f Dy: %3.0f", dx, dy);

        pv.attribs.translation_x.set(0, dx);
        pv.attribs.translation_y.set(0, dy);

        /* pv.attribs.off_x.set(0, dx); */
        /* pv.attribs.off_y.set(0, dy); */

        float scale = get_scale_factor(bbox);
        pv.attribs.scale_x.set(1, scale);
        pv.attribs.scale_y.set(1, scale);
        pv.attribs.alpha.set(get_view_normal_alpha(pv.view), 1.0);
     }

   void arrange_view(PixSwitcherView& pv, int index)
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Arrange View: %d", index);
        wlr_log(WLR_DEBUG, "\tSwitcher Box: %d %d %d %d",
                ps_box.x, ps_box.y, ps_box.width, ps_box.height);

        auto bbox = pv.view->get_bounding_box(pixswitcher_autobot);
        wlr_log(WLR_DEBUG, "\tView Box: %d %d %d %d",
                bbox.x, bbox.y, bbox.width, bbox.height);

        /* arrange_center_view(pv); */
        move_view(pv, index - 1);

        /* float dx = ps_box.x; */
        /* float dy = ps_box.y; */
        /* float dx = 300; */
        /* float dy = 300; */
        /* wlr_log(WLR_DEBUG, "\tDx: %3.0f Dy: %3.0f", dx, dy); */
        /* pv.attribs.translation_x.set(0, dx); */
        /* pv.attribs.translation_y.set(0, dy); */
        /* pv.attribs.off_x.set(0, dx); */
        /* pv.attribs.off_y.set(0, dy); */

        /* float scale = get_scale_factor(bbox); */
        /* pv.attribs.scale_x.set(1, scale); */
        /* pv.attribs.scale_y.set(1, scale); */
        /* pv.attribs.alpha.set(get_view_normal_alpha(pv.view), 1.0); */

        /* arrange_center_view(pv); */
        /* if (index == 1) return; */
        /* move_view(pv, index - 1); */
     }

   void cleanup_views(std::function<bool(PixSwitcherView&)> criteria)
     {
        auto it = views.begin();
        while (it != views.end())
          {
             if (criteria(*it))
               it = views.erase(it);
             else
               ++it;
          }

        count = views.size();
     }

   void cleanup_expired()
     {
        cleanup_views([=] (PixSwitcherView& pv)
                      { return view_expired(pv.index); });
     }

   void rebuild_view_list()
     {
        std::stable_sort(views.begin(), views.end(),
            [] (const PixSwitcherView& a, const PixSwitcherView& b)
        {
           enum category
             {
                FOCUSED   = 0,
                UNFOCUSED = 1,
                EXPIRED   = 2,
             };

           auto view_category = [] (const PixSwitcherView& pv)
             {
                if (pv.index == 1) // centered
                  return FOCUSED;

                if (view_expired(pv.index))
                  return EXPIRED;

                return UNFOCUSED;
             };

            category aCat = view_category(a), bCat = view_category(b);
            if (aCat == bCat)
             return a.index < b.index;
            else
             return aCat < bCat;
        });
     }

   wayfire_view invalidate_last_slot(int slot)
     {
        int i = 0;
        for (i = views.size() - 1; i >= 0; i--)
          {
             if (views[i].index == slot)
               {
                  move_view(views[i], slot - 1);
                  return views[i].view;
               }
          }

        return nullptr;
     }

   void fill_empty_slot(const int slot)
     {
        const int fslot = max_count - slot;

        auto vc = invalidate_last_slot(fslot);

        if (count_active_views() == 2)
          vc = get_unfocused_view();

        assert(vc);

        auto pv = create_view(vc);
        arrange_view(pv, slot);

        pv.to_end();
        pv.attribs.alpha.set(0, 1);
        views.push_back(std::move(pv));
     }

   void focus_next(int dir)
     {
        auto wviews = get_workspace_views();
        int size = wviews.size();
        int index = (size + dir) % size;
        auto fv = wviews[index];
        output->workspace->bring_to_front(fv);
     }

   void next_view(int dir)
     {
        int cleft = 0, cright = 0;

        cleanup_expired();

        //0, 1, 2;
        if (count_active_views() <= 1) return;

        int to_move = (1 << 1) | (1 << (1 - dir));
        for (auto& pv : views)
          {
             if (!view_expired(pv.index) && ((1 << pv.index) & to_move))
               {
                  to_move ^= (1 << pv.index);
                  move_view(pv, dir);
               }
             else if (!view_expired(pv.index))
               pv.refresh_start();

             cleft += (pv.index == 0);
             cright += (pv.index == 2);
          }

        if (bool(cleft) ^ bool(cright))
          {
             const int empty = 1 - dir;
             fill_empty_slot(empty);
          }

        rebuild_view_list();
        output->workspace->bring_to_front(views.front().view);
        duration.start();
     }

   void dim_bg(float dim)
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Dim Background: %f", dim);

        for (auto view : get_bg_views())
          {
             if (dim == 1.0)
               view->pop_transformer(pixswitcher_decepticon);
             else
               {
                  if (!view->get_transformer(pixswitcher_decepticon))
                    {
                       view->add_transformer(std::make_unique<wf::view_2D>(view),
                                             pixswitcher_decepticon);
                    }

                  auto tr = dynamic_cast<wf::view_2D*>
                    (view->get_transformer(pixswitcher_decepticon).get());
                  /* tr->color[0] = tr->color[1] = tr->color[2] = dim; */
                  tr->alpha = dim;
               }
          }
     }

   float get_scale_factor(const wf::geometry_t& box) const
     {
        constexpr float sp = 0.5;
        auto og = output->get_relative_geometry();
        float mw = og.width * sp;
        float mh = og.height * sp;
        float exact = std::min(mw / box.width, mh / box.height);
        return std::min(exact, 1.0f) * 0.25;//thumb_scale;
     }

   void render_view(PixSwitcherView& pv, const wf::framebuffer_t& fb)
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Render View");

        auto transform = dynamic_cast<wf::view_2D*>
          (pv.view->get_transformer(pixswitcher_autobot).get());
        assert(transform);

        /* transform->translation = */
        /*   glm::translate(glm::mat4(1.0), */
        /*                  {(double)pv.attribs.translation_x, */
        /*                       (double)pv.attribs.translation_y, 1.0}); */

        /* transform->scaling = */
        /*   glm::scale(glm::mat4(1.0), */
        /*              {(double)pv.attribs.scale_x, */
        /*              (double)pv.attribs.scale_y, 1.0}); */

        transform->translation_x = pv.attribs.translation_x;
        transform->translation_y = pv.attribs.translation_y;

        /* transform->off_x = pv.attribs.off_x; */
        /* transform->off_y = pv.attribs.off_y; */

        wlr_log(WLR_DEBUG, "\tTrans: %3.0f %3.0f",
                transform->translation_x,
                transform->translation_y);

        transform->scale_x = pv.attribs.scale_x;
        transform->scale_y = pv.attribs.scale_y;
        transform->alpha = pv.attribs.alpha;

        /* transform->color[3] = pv.attribs.alpha; */
        pv.view->render_transformed(fb, fb.geometry);
     }

   void render_ps_rect(const wf::framebuffer_t& fb)
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Render Rect");

        wf::color_t color
          {
             0.2, 0.2, 0.2, 0.13 //0.87
          };

        if (!runtime_config.use_pixman)
          {
             OpenGL::render_begin(fb);
             OpenGL::render_rectangle(ps_box, color, fb.get_orthographic_projection());
             OpenGL::render_end();
          }
        else
          {
             Pixman::render_begin(fb);
             Pixman::render_rectangle(ps_box, color, fb.get_orthographic_projection());
             Pixman::render_end();
          }
     }

   wf::render_hook_t pixswitcher_renderer = [=] (const wf::framebuffer_t& fb)
     {
        wlr_log(WLR_DEBUG, "PixSwitcher Render Fb");

        if (!runtime_config.use_pixman)
          {
             OpenGL::render_begin(fb);
             OpenGL::clear({0, 0, 0, 1});
             OpenGL::render_end();
          }
        else
          {
             Pixman::render_begin(fb);
             Pixman::clear({0, 0, 0, 1});
             Pixman::render_end();
          }

        dim_bg(bg_dim);

        for (auto view : get_bg_views())
          view->render_transformed(fb, fb.geometry);

        render_ps_rect(fb);

        wlr_log(WLR_DEBUG, "\tNum of Views: %d", views.size());

        for (auto& view : wf::reverse(views))
          render_view(view, fb);

        for (auto& view : get_overlay_views())
          view->render_transformed(fb, fb.geometry);

        if (!duration.running())
          {
             cleanup_expired();

             if (!active)
               deinit_switcher();
          }
     };
};

DECLARE_WAYFIRE_PLUGIN(PixSwitcher);

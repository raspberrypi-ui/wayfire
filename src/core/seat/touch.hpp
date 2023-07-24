#ifndef TOUCH_HPP
#define TOUCH_HPP

#include <map>
#include <wayfire/touch/touch.hpp>
#include "wayfire/util.hpp"
#include "wayfire/view.hpp"
#include <wayfire/signal-definitions.hpp>

#include "surface-map-state.hpp"

// TODO: tests
namespace wf
{
struct plugin_grab_interface_t;
using input_surface_selector_t =
    std::function<wf::surface_interface_t*(const wf::pointf_t&, wf::pointf_t&)>;

/**
 * Responsible for managing touch gestures and forwarding events to clients.
 */
class touch_interface_t : public noncopyable_t
{
  public:
    touch_interface_t(wlr_cursor *cursor, wlr_seat *seat,
        input_surface_selector_t surface_at);
    ~touch_interface_t();

    /** Get the positions of the fingers */
    const touch::gesture_state_t& get_state() const;

    /** Get the focused surface */
    wf::surface_interface_t *get_focus() const;

    /**
     * Set the active grab interface.
     *
     * If a grab interface is active, all input events will be sent to it
     * instead of the client.
     */
    void set_grab(wf::plugin_grab_interface_t *grab);

    /**
     * Register a new touchscreen gesture.
     */
    void add_touch_gesture(nonstd::observer_ptr<touch::gesture_t> gesture);

    /**
     * Unregister a touchscreen gesture.
     */
    void rem_touch_gesture(nonstd::observer_ptr<touch::gesture_t> gesture);

  private:
    wlr_seat *seat;
    wlr_cursor *cursor;
    input_surface_selector_t surface_at;
    wf::plugin_grab_interface_t *grab = nullptr;

    wf::wl_listener_wrapper on_down, on_up, on_motion, on_cancel, on_frame;
    void handle_touch_down(int32_t id, uint32_t time, wf::pointf_t current,
        input_event_processing_mode_t mode);
    void handle_touch_motion(int32_t id, uint32_t time, wf::pointf_t current,
        bool real_event, input_event_processing_mode_t mode);
    void handle_touch_up(int32_t id, uint32_t time,
        input_event_processing_mode_t mode);

    void set_touch_focus(wf::surface_interface_t *surface,
        int32_t id, uint32_t time, wf::pointf_t current);

    touch::gesture_state_t finger_state;

    /** Pressed a finger on a surface and dragging outside of it now */
    wf::surface_interface_t *grabbed_surface = nullptr;
    wf::surface_interface_t *focus = nullptr;
    void start_touch_down_grab(wf::surface_interface_t *surface);
    void end_touch_down_grab();

    void update_gestures(const wf::touch::gesture_event_t& event);
    std::vector<nonstd::observer_ptr<touch::gesture_t>> gestures;

    SurfaceMapStateListener on_surface_map_state_change;
    wf::signal_connection_t on_stack_order_changed;

    std::unique_ptr<touch::gesture_t> multiswipe, edgeswipe, multipinch;
    void add_default_gestures();

    /** Enable/disable cursor depending on how many touch points are there */
    void update_cursor_state();
};
}

#endif /* end of include guard: TOUCH_HPP */

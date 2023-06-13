#pragma once

#include <wayfire/nonstd/noncopyable.hpp>
#include <wayfire/object.hpp>
#include <wayfire/output.hpp>
#include <wayfire/geometry.hpp>
#include <wayfire/render-manager.hpp>
#include <wayfire/workspace-stream.hpp>
#include <wayfire/workspace-manager.hpp>
#include "../main.hpp"

namespace wf
{
/**
 * A class which holds one workspace stream per workspace on the given output.
 *
 * Using this interface allows all plugins to use the same OpenGL textures for
 * the workspaces, thereby reducing the memory overhead of a workspace stream.
 */
class workspace_stream_pool_t : public noncopyable_t, public wf::custom_data_t
{
  public:
    /**
     * Make sure there is a stream pool object on the given output, and
     * increase its reference count.
     */
    static nonstd::observer_ptr<workspace_stream_pool_t> ensure_pool(
        wf::output_t *output)
    {
        if (!output->has_data<workspace_stream_pool_t>())
        {
            output->store_data(std::unique_ptr<workspace_stream_pool_t>(
                new workspace_stream_pool_t(output)));
        }

        auto pool = output->get_data<workspace_stream_pool_t>();
        ++pool->ref_count;

        return pool;
    }

    /**
     * Decrease the reference count, and if no more references are being held,
     * then destroy the pool object.
     */
    void unref()
    {
        --ref_count;
        if (ref_count == 0)
        {
            output->erase_data<workspace_stream_pool_t>();
        }
    }

    ~workspace_stream_pool_t()
    {
       if (!runtime_config.use_pixman)
         OpenGL::render_begin();

        for (auto& row : this->streams)
        {
            for (auto& stream : row)
            {
                stream.buffer.release();
            }
        }

       if (!runtime_config.use_pixman)
         OpenGL::render_end();
    }

    /**
     * Get the workspace stream for the given workspace
     */
    wf::workspace_stream_t& get(wf::point_t workspace)
    {
        return streams[workspace.x][workspace.y];
    }

    /**
     * Update the contents of the given workspace.
     *
     * If the workspace has not been started before, it will be started.
     */
    void update(wf::point_t workspace)
    {
        auto& stream = get(workspace);
        if (stream.running)
        {
            output->render->workspace_stream_update(stream);
        } else
        {
            output->render->workspace_stream_start(stream);
        }
    }

    /**
     * Stop the workspace stream.
     */
    void stop(wf::point_t workspace)
    {
        auto& stream = get(workspace);
        if (stream.running)
        {
            output->render->workspace_stream_stop(stream);
        }
    }

  private:
    workspace_stream_pool_t(wf::output_t *output)
    {
        this->output = output;

        auto wsize = this->output->workspace->get_workspace_grid_size();
        this->streams.resize(wsize.width);
        for (int i = 0; i < wsize.width; i++)
        {
            this->streams[i].resize(wsize.height);
            for (int j = 0; j < wsize.height; j++)
            {
                this->streams[i][j].ws = {i, j};
            }
        }
    }

    /** Number of active users of this instance */
    uint32_t ref_count = 0;

    wf::output_t *output;
    std::vector<std::vector<wf::workspace_stream_t>> streams;
};
}

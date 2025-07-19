#include <iostream>

#include <ngin/job/ngin.h>

#include <ngin/debug/manager.h>
#include <ngin/asset/manager.h>
#include <ngin/render/manager.h>
#include <ngin/scene/object/manager.h>

/**
 * @brief The main application class for the Ngin engine.
 *
 * This class orchestrates the initialization, update loop, and cleanup
 * of various Ngin engine components, including job management, asset management,
 * rendering, and scene object management.
 */
class NginApplication {
public:
    /**
     * @brief Constructs an NginApplication.
     *
     * Initializes the application without starting the engine loop.
     */
    NginApplication() {}

    /**
     * @brief Runs the main application loop.
     *
     * This method initializes the engine components, enters the update loop,
     * and performs necessary cleanup before exiting.
     */
    void run() {
        init_();
        update_();
        cleanup_();
    }

private:
    ngin::jobs::JobNgin job_ngin_; ///< @brief Job manager for asynchronous tasks.

    ngin::asset::AssetManager asset_mgr_;   ///< @brief Manages loading and accessing assets.
    ngin::render::RenderManager render_mgr_; ///< @brief Handles all rendering operations.

    // temp
    ngin::scene::ObjectManager object_mgr_; ///< @brief Manages scene objects (temporary).

    /**
     * @brief Initializes all core Ngin engine components.
     *
     * This includes setting up asset management, processing initial asset jobs,
     * initializing the render manager, and building initial scene objects.
     */
    void init_() {

        // Asset setup
            JobHandle asset_setup_handle = asset_mgr_.process_setup_jobs(job_ngin_);
            job_ngin_.wait_for(asset_setup_handle);
            asset_mgr_.debug_show();
            asset_mgr_.log_snapshot();
        
        // Render setup
            render_mgr_.setup();

        // Scene setup
            std::string object_origin = "sphere";
            // @todo Consider making "sphere" a configurable default or loaded from a scene file.
            ObjectAsset* object_asset = asset_mgr_.get<ObjectAsset>("object", object_origin);
            if (object_asset) {
                object_mgr_.build_from_asset(*object_asset);
            } else {
                // @warning Log an error if the object asset isn't found.
                std::cerr << "Warning: Object asset '" << object_origin << "' not found." << std::endl;
            }
    }

    /**
     * @brief The main application update loop.
     *
     * This loop continues as long as the render manager indicates that the
     * application window should not close. It performs early and late
     * rendering updates.
     */
    void update_() {
        while (!render_mgr_.should_close()) {
            render_mgr_.update_early();

            // @todo Add game logic, input processing, physics updates here.
            


            render_mgr_.update_late();
        }
    }

    /**
     * @brief Cleans up Ngin engine components.
     *
     * This method is called after the main update loop finishes to
     * release resources, primarily by cleaning up the render manager.
     */
    void cleanup_() {
        render_mgr_.cleanup();
    }
};

/**
 * @brief The main entry point of the Ngin application.
 *
 * Initializes and runs the NginApplication, handling any exceptions
 * that may occur during execution.
 *
 * @return EXIT_SUCCESS if the application runs successfully, EXIT_FAILURE otherwise.
 */
int main() {
    NginApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
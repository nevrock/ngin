#include <iostream>

#include <ngin/job/ngin.h>

#include <ngin/debug/manager.h>
#include <ngin/asset/manager.h>
#include <ngin/render/manager.h>
#include <ngin/scene/object/manager.h>

class NginApplication {
public:
    NginApplication() {}

    void run() {
        init_();
        update_();
        cleanup_();
    }

private:
    ngin::jobs::JobNgin job_ngin_;

    ngin::asset::AssetManager asset_mgr_;
    ngin::render::RenderManager render_mgr_;
    ngin::debug::DebugManager debug_mgr_;

    // temp
    ngin::scene::ObjectManager object_mgr_;

    void init_() {

        // Asset setup
            JobHandle asset_setup_handle = asset_mgr_.process_setup_jobs(job_ngin_, debug_mgr_.get_context());
            job_ngin_.wait_for(asset_setup_handle);

            debug_mgr_.show();
            asset_mgr_.log_snapshot();
        
        // Render setup
            render_mgr_.setup();

        // Scene setup
            std::string object_origin = "sphere";
            ObjectAsset* object_asset = asset_mgr_.get<ObjectAsset>("object", object_origin);
            if (object_asset) {
                object_mgr_.build_from_asset(*object_asset);
            }
    }
    void update_() {
        while (!render_mgr_.should_close()) {
            render_mgr_.update_early();


            render_mgr_.update_late();
        }
    }
    void cleanup_() {
        render_mgr_.cleanup();
    }
};

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
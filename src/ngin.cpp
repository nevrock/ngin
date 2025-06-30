#include <ngin/ngin.h>

#include <iostream>

#include <ngin/game/game.h>
#include <ngin/render/renderer.h>

#include <ngin/job/system.h>

class NginApplication {
public:
    NginApplication() : renderer_() {}

    void run() {
        init_();
        run_();
        cleanup_();
    }

private:
    Assets assets_;
    Game game_;
    Renderer renderer_;

    ngin::jobs::JobSystem jobs_;

    void init_() {
        // game_.setup();
        renderer_.setup();
    }
    void run_() {
        while (!renderer_.should_close()) {
            // game_.update();
            renderer_.update();
        }
    }
    void cleanup_() {
        // game_.cleanup();
        renderer_.cleanup();
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
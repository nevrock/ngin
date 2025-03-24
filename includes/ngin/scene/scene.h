#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <ngin/scene/object.h>
#include <ngin/resources.h>
#include <ngin/log.h>

class Scene {
public:
    Scene(const std::string& pathToSceneFile) : path_(pathToSceneFile) {
    }
    ~Scene() = default;

    void build() {
        // load
        //auto lex = Resources::loadLexicon(path_);
        auto lex = Resources::loadObject(path_);
        //lex.print();
        std::string name = lex.getC<std::string>("name", "default");
        origin_ = std::make_unique<Object>(name, lex);
        origin_->build();

        log();
    }

    void launch() {
        origin_->launch();
    }
    void updateLogic() {
        origin_->updateLogic();
    }
    void updateTransform() {
        origin_->updateTransform();
    }

    void log(int indent = 0) const {
        origin_->log(indent);
    }

private:
    std::string path_;
    std::unique_ptr<Object> origin_;
};


#endif // SCENE_H

#ifndef GL_DEPTH_TEST_H
#define GL_DEPTH_TEST_H

class GlDepthTest : public Node {
public:
    explicit GlDepthTest(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary), isEnable_(dictionary.getC<bool>("is_enable", true))
    {
    }
    ~GlDepthTest() override = default;

    void update(std::string& pass) override {
        Node::update(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
        if (isEnable_) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

        } else {
            glDisable(GL_DEPTH_TEST);
        }
    }
    void setup() override {
        Node::setup();
    }

protected:
    bool isEnable_;
};

#endif 
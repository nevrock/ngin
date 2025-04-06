#ifndef NGIN_H
#define NGIN_H

#include <memory>
#include <functional>
#include <atomic>

#include <ngin/log.h>
#include <ngin/lex.h>
#include <ngin/resources.h>

class Ngin {
public:

    static void init() {
        env_ = std::make_shared<Lex>(Resources::loadLexicon("env"));
    }

    static void setEnv(std::shared_ptr<Lex> nevf) {
        env_ = nevf;
    }
    static std::shared_ptr<Lex> getEnv() {
        return env_;
    }
    
    template <typename T>
    static T envget(const std::string& key) { 
        return env_->getC<T>(key, T());
    }
    template <typename T>
    static void envset(const std::string& key, T value) {
        env_->set(key, value);
    }

private:
    static inline std::shared_ptr<Lex> env_;

};

#endif // NGIN_H
#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include <ngin/debug/logger.h>

#include <ngin/job/ngin.h>

#include <ngin/scene/module/bucket.h>


namespace ngin {
namespace scene {

class ModuleManager {
public:
    // Constructor to initialize modules
    ModuleManager() : logger_(new ngin::debug::Logger("ModuleManager")) {
        setup_buckets_();
    }
    ~ModuleManager() {
        delete logger_;
        for (auto& bucket : buckets_) {
            delete bucket.second;
        }
    }

    template<typename T>
    T* get(const std::string& bucket, const std::string& name) {
        if (buckets_.find(bucket) == buckets_.end()) {
            return nullptr;
        }
        return buckets_[bucket]->get<T>(name);
    }

    void log_snapshot() {
        logger_->info("Module Snapshot:");
        for (auto& bucket : buckets_) {
            logger_->info(
                "Module Bucket " + bucket.second->get_name() + ", loaded: " + std::to_string(bucket.second->get_module_count()) + " modules",
                "", 1
            );
        }
    }

private:
    ngin::debug::Logger* logger_;
    std::unordered_map<std::string, ModuleBucket*> buckets_;

    void setup_buckets_() {

    }
};

}
}

#endif // MODULE_MANAGER_H
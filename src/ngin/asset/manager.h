#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <ngin/debug/logger.h>
#include <ngin/debug/bucket.h>

#include <ngin/job/ngin.h>

#include <ngin/asset/bucket.h>


namespace ngin {
namespace asset {

class AssetManager {
public:
    // Constructor to initialize assets
    AssetManager() : logger_(new ngin::debug::Logger("AssetManager")) {
        setup_buckets_();
    }
    ~AssetManager() {
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

    JobHandle process_setup_jobs(ngin::jobs::JobNgin& job_ngin) {
        logger_->info("Asset processing setup jobs");

        ngin::debug::Printer& printer = debugger_.get_context();

        std::vector<std::function<void()>> bucket_setup_tasks;
        for (auto& bucket : buckets_) {
            bucket_setup_tasks.push_back([&, bucket_ptr = bucket.second]() {
                bucket_ptr->setup(printer);
            });
        }
        JobHandle buckets_setup_handle = job_ngin.submit_jobs(bucket_setup_tasks, JobType::AssetSetup);
        job_ngin.wait_for(buckets_setup_handle);

        std::vector<std::function<void()>> preload_tasks;
        for (auto& bucket : buckets_) {
            for (auto& job : bucket.second->generate_preload_jobs(printer)) {
                preload_tasks.push_back(job);
            }
        }
        JobHandle preload_handle = job_ngin.submit_jobs(preload_tasks, JobType::AssetLoading);

        return preload_handle;
    }
    void debug_show() {
        debugger_.show();
    }
    void process_update_jobs() {

    }
    void process_cleanup_jobs() {

    }

    void log_snapshot() {
        logger_->info("Asset Snapshot:");
        for (auto& bucket : buckets_) {
            logger_->info(
                "Asset Bucket " + bucket.second->get_name() + ", loaded: " + std::to_string(bucket.second->get_asset_count()) + " assets",
                "", 1
            );
        }
    }

private:
    ngin::debug::Logger* logger_;
    ngin::debug::DebugBucket debugger_;
    std::unordered_map<std::string, AssetBucket*> buckets_;

    void setup_buckets_() {
        // Create the mesh bucket
        std::string mesh_bucket_name = "mesh";
        AssetBucket* mesh_bucket = new AssetBucket(mesh_bucket_name);
        buckets_[mesh_bucket_name] = mesh_bucket;

        // Create the object bucket
        std::string object_bucket_name = "object";
        AssetBucket* object_bucket = new AssetBucket(object_bucket_name);
        buckets_[object_bucket_name] = object_bucket;

        // Create the shader bucket
        std::string shader_bucket_name = "shader";
        AssetBucket* shader_bucket = new AssetBucket(shader_bucket_name);
        buckets_[shader_bucket_name] = shader_bucket;
    }
};

}
}

#endif // ASSET_MANAGER_H
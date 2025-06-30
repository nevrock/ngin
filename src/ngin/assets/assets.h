#ifndef ASSETS_H
#define ASSETS_H

#include <ngin/assets/bucket.h>
#include <ngin/debug/logger.h>

#include <string>
#include <unordered_map>
#include <memory> // Include memory header for std::enable_shared_from_this
#include <functional>
#include <string>
#include <map> // Include for std::map
#include <vector>

class Assets {
public:
    // Constructor to initialize assets
    Assets() : logger_(new Logger("Assets")) {
        logger_->info("Initializing assets");

        setup_buckets();
    }
    ~Assets() {
        for (auto& bucket : buckets_) {
            delete bucket.second;
        }
        buckets_.clear();
        delete logger_;
    }

    template<typename T>
    T* get(const std::string& bucket, const std::string& name) {
        if (buckets_.find(bucket) == buckets_.end()) {
            buckets_[bucket] = new AssetsBucket(bucket);
        }
        return buckets_[bucket]->get<T>(name);
    }
    void load_asset(const std::string& bucket, const std::string& name) {
        if (buckets_.find(bucket) == buckets_.end()) {
            buckets_[bucket] = new AssetsBucket(bucket);
        }
        buckets_[bucket]->load(name);
    }

    std::vector<std::string> get_buckets() {
        std::vector<std::string> buckets;
        for (auto& bucket : buckets_) {
            buckets.push_back(bucket.first);
        }
        return buckets;
    }
    std::vector<std::string> get_asset_names_for_bucket(const std::string& bucket) {
        if (buckets_.find(bucket) == buckets_.end()) {
            return std::vector<std::string>();
        }
        return buckets_[bucket]->get_asset_names();
    }


private:
    std::map<std::string, AssetsBucket*> buckets_;
    Logger* logger_;

    void setup_buckets() {
        logger_->info("Setting up asset buckets");

        // Create the data bucket
        logger_->info("Creating data asset bucket", 0);
        std::string data_bucket_name = "data";
        AssetsBucket* data_bucket = new AssetsBucket(data_bucket_name);
        data_bucket->setup();
        buckets_[data_bucket_name] = data_bucket;

        // Create the mesh bucket
        logger_->info("Creating mesh asset bucket", 0);
        std::string mesh_bucket_name = "mesh";
        AssetsBucket* mesh_bucket = new AssetsBucket(mesh_bucket_name);
        mesh_bucket->setup();
        buckets_[mesh_bucket_name] = mesh_bucket;

        // Create the object bucket
        logger_->info("Creating object asset bucket", 01);
        std::string object_bucket_name = "object";
        AssetsBucket* object_bucket = new AssetsBucket(object_bucket_name);
        object_bucket->setup();
        buckets_[object_bucket_name] = object_bucket;

        // Create the shader bucket
        logger_->info("Creating shader asset bucket", 01);
        std::string shader_bucket_name = "shader";
        AssetsBucket* shader_bucket = new AssetsBucket(shader_bucket_name);
        shader_bucket->setup();
        buckets_[shader_bucket_name] = shader_bucket;
    }
};

#endif // ASSETS_H
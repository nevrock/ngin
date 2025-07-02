#ifndef BUCKET_H
#define BUCKET_H

#include <ngin/asset/asset.h>

#include <ngin/util/file.h>
#include <ngin/util/id.h>
#include <ngin/atlas/atlas.h>

#include <ngin/debug/logger.h>

#include <ngin/job/collections/vector.h>
#include <ngin/job/collections/map.h>

#include <string>
#include <unordered_map>
#include <memory> // Include memory header for std::enable_shared_from_this
#include <functional>
#include <string>
#include <tuple>
#include <vector>

namespace ngin {
namespace asset {

struct AssetData {
    std::string name;
    std::string kind;
    std::string location;
    bool preload;

    void from_atlas(Atlas& atlas) {
        name = *atlas.get<std::string>("name");
        kind = *atlas.get<std::string>("kind");
        location = *atlas.get<std::string>("location");
        preload = *atlas.get<bool>("preload");
    }
};

struct AssetManifest {
    ngin::jobs::ParallelMap<std::string, AssetData> data;

    void from_atlas(Atlas& atlas) {
        for (auto& asset : atlas.getmap()) {
            Atlas* asset_manifest = atlas.get<Atlas>(asset.first);
            if (!asset_manifest) {
                continue;
            }
            AssetData asset_data;
            asset_data.from_atlas(*asset_manifest);
            data.add(asset.first, asset_data);
        }
    }
    std::vector<std::string> keys() {
        return data.keys();
    }
    std::optional<AssetData> get(const std::string& name) { // Changed return to std::optional
        return data.get(name); // ParallelMap::get already returns std::optional<AssetData>
    }
};

class AssetBucket {
public:

    AssetBucket(const std::string& name) : name_(name) {
    }
    ~AssetBucket() {
    }

    std::string& get_name() {
        return name_;
    }

    std::vector<std::function<void()>> generate_setup_job(ngin::debug::Printer& debug) {
        return std::vector<std::function<void()>> {
            [this, &debug]() {
                setup(debug);
            }
        };
    }
    void setup(ngin::debug::Printer& debug) {
        debug.info("Setting up assets bucket: " + name_, debug_name_);

        std::tuple<std::string, bool> asset_path = FileUtil::get_asset_path(name_ + "/manifest.atl");
        std::tuple<std::string, bool> resource_path = FileUtil::get_resource_path(name_ + "/manifest.atl");

        Atlas* manifest = new Atlas();

        if (std::get<1>(asset_path)) {
            debug.info("Found assets manifest file at: " + std::get<0>(asset_path),debug_name_);
            manifest->read(std::get<0>(asset_path));
            if (std::get<1>(resource_path)) {
                debug.info("Found resources manifest file at: " + std::get<0>(resource_path), debug_name_);
                Atlas* other = new Atlas();
                other->read(std::get<0>(resource_path));
                manifest->sync(other);
            } 
        } else {
            if (std::get<1>(resource_path)) {
                debug.info("Found resources manifest file at: " + std::get<0>(resource_path), debug_name_);

                manifest->read(std::get<0>(resource_path));
            }
        }

        manifest_.from_atlas(*manifest);
        delete manifest;
    }

    std::vector<std::function<void()>> generate_preload_jobs(ngin::debug::Printer& debug) {
        std::vector<std::function<void()>> jobs;
        for (const auto& asset : manifest_.data) {

            std::optional<AssetData> asset_data = manifest_.get(asset.first);
            if (!asset_data) {
                continue;
            }
            AssetData* asset_data_val = &asset_data.value();

            if (asset_data_val->preload) {
                jobs.push_back(generate_asset_load_job(asset.first, debug));
            }
        }
        return jobs;
    }
    std::function<void()> generate_asset_load_job(const std::string& asset_name, ngin::debug::Printer& debug) {
        return [this, asset_name, &debug]() {
            load(asset_name, debug);
        };
    }
    void load(const std::string& asset_name, ngin::debug::Printer& debug) {
        std::optional<AssetData> asset_data = manifest_.get(asset_name);
        if (!asset_data) {
            return;
        }
        
        debug.info("Loading asset: " + asset_name + ", of kind: " + asset_data->kind + ", at location: " + asset_data->location, debug_name_);

        load_asset_(asset_name, asset_data->kind, asset_data->location, debug);
    }
    void unload(const std::string& asset_name) {
        unload_asset_(asset_name);
    }

    template<typename T>
    T* get(const std::string& name) {
        // Try to get by name_to_id_mapping_ first, which is more efficient
        std::optional<unsigned int> asset_id_opt = name_to_id_mapping_.get(name);

        if (asset_id_opt) {
            std::optional<std::shared_ptr<Asset>> asset_ptr_opt = assets_.get(asset_id_opt.value());
            if (asset_ptr_opt) {
                // Asset found by ID
                T* casted_asset = dynamic_cast<T*>(asset_ptr_opt.value().get()); // Get raw pointer from shared_ptr
                if (casted_asset) {
                    return casted_asset;
                }
                // If dynamic_cast fails, it means the type T is not compatible.
                // Log an error or return nullptr as appropriate.
                logger_->error("Asset '" + name + "' found but not of requested type.");
                return nullptr;
            }
        }
        logger_->warn("Asset '" + name + "' not found or could not be loaded/casted.");
        return nullptr; // Asset not found, or could not be casted, or not loaded
    }
    std::vector<std::string> get_asset_names() {
        return manifest_.keys();
    }
    unsigned int get_asset_count() {
        return assets_.size();
    }


    using AssetFactory = std::function<std::shared_ptr<Asset>(unsigned int id, const std::string& name)>;
    static ngin::jobs::ParallelMap<std::string, AssetFactory>& get_asset_factories() {
        static ngin::jobs::ParallelMap<std::string, AssetBucket::AssetFactory> asset_factories_;
        return asset_factories_;
    }
    static void register_asset(const std::string& name, AssetFactory factory) {
        get_asset_factories().add(name, factory);
    }

private:
    ngin::debug::Logger* logger_ = new ngin::debug::Logger("AssetBucket");
    std::string name_;

    AssetManifest manifest_;
    ngin::jobs::ParallelMap<unsigned int, std::shared_ptr<Asset>> assets_;
    ngin::jobs::ParallelMap<std::string, unsigned int> name_to_id_mapping_;

    std::string debug_name_ = "AssetBucket::";
    
    void load_asset_(const std::string& asset_name, std::string& type, std::string& location, ngin::debug::Printer& debug) {
        // logger_->info("Loading asset: " + asset_name + ", at location: " + location, 1);
        std::optional<AssetBucket::AssetFactory> factory = get_asset_factories().get(type);
        if (factory) {
            unsigned int id = IdUtil::get_unique_id(); // Generate a unique ID for the asset
            std::shared_ptr<Asset> asset = (*factory)(id, asset_name); // Dereference the optional to call the factory
            std::tuple<std::string, bool> asset_path = FileUtil::get_generic_asset_path(location);
            if (std::get<1>(asset_path)) {
                asset->read(std::get<0>(asset_path), debug);

                assets_.add(id, asset);
                name_to_id_mapping_.add(asset_name, id);
            } else {
                // logger_->info("Asset file not located, did not load asset: " + asset_name);
            }
        } else {
            // logger_->info("Asset type not found, did not load asset: " + asset_name);            
        }
    }
    void unload_asset_(const std::string& asset_name) {
        std::optional<unsigned int> asset_id_opt = name_to_id_mapping_.get(asset_name);
        if (asset_id_opt) {
            unsigned int asset_id = asset_id_opt.value();
            assets_.remove(asset_id);
            name_to_id_mapping_.remove(asset_name);
        }
    }
};

}
}

#endif // BUCKET_H

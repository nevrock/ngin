#ifndef BUCKET_H
#define BUCKET_H

#include <ngin/assets/asset.h>
#include <ngin/util/file.h>
#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

#include <string>
#include <unordered_map>
#include <memory> // Include memory header for std::enable_shared_from_this
#include <functional>
#include <string>
#include <tuple>

class AssetsBucket {
public:

    AssetsBucket(const std::string& name) : name_(name) {
        logger_ = new Logger("AssetsBucket::" + name);
        manifest_ = new Atlas();
    }
    ~AssetsBucket() {
        delete manifest_;
        for (auto& asset : assets_) {
            delete asset;
        }
        delete(logger_);
    }

    void setup() {
        logger_->info("Setting up assets bucket: " + name_);

        std::tuple<std::string, bool> asset_path = FileUtil::get_asset_path(name_ + "/manifest.atl");
        std::tuple<std::string, bool> resource_path = FileUtil::get_resource_path(name_ + "/manifest.atl");

        if (std::get<1>(asset_path)) {
            logger_->info("Found assets manifest file at: " + std::get<0>(asset_path), 1);
            manifest_->read(std::get<0>(asset_path));
            if (std::get<1>(resource_path)) {
                logger_->info("Found resources manifest file at: " + std::get<0>(resource_path), 1);
                Atlas* other = new Atlas();
                other->read(std::get<0>(resource_path));
                manifest_->sync(other);
            } 
        } else {
            if (std::get<1>(resource_path)) {
                logger_->info("Found resources manifest file at: " + std::get<0>(resource_path), 1);

                manifest_->read(std::get<0>(resource_path));
            }
        }

        logger_->info("Setting up assets bucket: " + name_, 1);

        for (const auto& asset : manifest_->getmap()) {

            Atlas* asset_manifest = manifest_->get<Atlas>(asset.first);
            
            bool* preload = asset_manifest->get<bool>("preload");
            if (!preload) {
                preload = new bool(false);
            }

            if (*preload) {
                logger_->info("Preloading asset: " + asset.first, 0);
                load(asset.first);
            }
        }
    }
    void load(const std::string& asset_name) {
        bool has_asset = false;
        for (auto& asset : assets_) {
            if (asset->get_name() == asset_name) {
                has_asset = true;
            }
        }
        if (has_asset) {
            return;
        }

        Atlas* asset_manifest = manifest_->get<Atlas>(asset_name, nullptr);
        if (!asset_manifest) {
            return;
        }
        std::string* type = asset_manifest->get<std::string>("kind");
        std::string* location = asset_manifest->get<std::string>("location");
        if (!type || !location) {
            return;
        }

        load_asset_(asset_name, *type, *location);
    }
    void unload(std::string& asset_name) {
        for (auto it = assets_.begin(); it != assets_.end(); ++it) {
            if ((*it)->get_name() == asset_name) {
                assets_.erase(it);
                return;
            }
        }
    }

    template<typename T>
    T* get(const std::string& name, bool force_load=true) {
        for (auto& asset : assets_) {
            if (asset->get_name() == name) {
                const T* casted_asset = dynamic_cast<const T*>(asset);
                if (casted_asset) {
                    return const_cast<T*>(casted_asset);            
                }
            }
        }
        if (force_load) {
            load(name);
            for (auto& asset : assets_) {
                if (asset->get_name() == name) {
                    const T* casted_asset = dynamic_cast<const T*>(asset);
                    if (casted_asset) {
                        return const_cast<T*>(casted_asset);            
                    }
                }
            }
        }
        return nullptr;
    }
    std::vector<std::string> get_asset_names() {
        return manifest_->keys();
    }


    using AssetFactory = std::function<Asset*(const std::string& name)>;
    static std::unordered_map<std::string, AssetFactory>& get_asset_factories() {
        static std::unordered_map<std::string, AssetsBucket::AssetFactory> asset_factories;
        return asset_factories;
    }
    static void register_asset(const std::string& name, AssetFactory factory) {
        get_asset_factories()[name] = factory;
    }

private:
    std::string name_;
    Atlas* manifest_;
    std::vector<Asset*> assets_;

    std::unordered_map<std::string, std::function<void()>> asset_factories;

    Logger* logger_;

    void load_asset_(const std::string& asset_name, std::string& type, std::string& location) {
        logger_->info("Loading asset: " + asset_name + ", at location: " + location, 1);

        auto it = get_asset_factories().find(type);
        if (it != get_asset_factories().end()) {
            Asset* asset = it->second(asset_name);
            std::tuple<std::string, bool> asset_path = FileUtil::get_generic_asset_path(location);
            if (std::get<1>(asset_path)) {
                asset->read(std::get<0>(asset_path));
                assets_.push_back(std::move(asset));
            } else {
                logger_->info("Asset file not located, did not load asset: " + asset_name);
            }
        } else {
            logger_->info("Asset type not found, did not load asset: " + asset_name);            
        }
    }
};

#endif // BUCKET_H

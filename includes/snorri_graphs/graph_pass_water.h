#ifndef GRAPH_PASS_WATER_H
#define GRAPH_PASS_WATER_H

#include <snorri_graphs/graph_data.h>
#include <snorri/dict.h>
#include <snorri/log.h>
#include <snorri/noise.h>
#include <vector>
#include <queue>
#include <unordered_set>

class GraphPassWater {
public:
    GraphPassWater() {}

    void executePass(std::vector<std::shared_ptr<GraphCenter>>& centers,
                 std::vector<std::shared_ptr<GraphCorner>>& corners,
                 std::vector<std::shared_ptr<GraphEdge>>& edges, Dict* config) {
        Log::console("graph pass water!");

        // Initialize water properties based on noise or border status
        for (const auto& corner : corners) {
            double nx = corner->point.x;
            double ny = corner->point.y;
            Dict* d = config->get<Dict>("noise_island");
            Noise noise(d);
            float noiseVal = noise.getNoise(nx, ny);

            corner->water = (noiseVal > config->getC<float>("threshold_water", 0.0)) || corner->border;
        }

        // Set water properties for centers based on adjacent corners
        for (const auto& center : centers) {
            if (center->border) {
                center->water = true;
                continue;
            }
            int waterCount = 0;
            for (const auto& corner : center->corners) {
                if (corner->water || corner->border) {
                    waterCount++;
                }
            }
            // Log::console("graph water pass, center water count: " + std::to_string(waterCount));
            if (waterCount > config->getC<int>("threshold_water_count", 2)) {
                center->water = true;
                center->ocean = false;  // Assume it's a lake initially
            }
        }

        // Flood fill to identify ocean centers starting from border water centers
        std::queue<std::shared_ptr<GraphCenter>> queue;
        for (const auto& center : centers) {
            if (center->border && center->water) {
                center->ocean = true;
                queue.push(center);
            }
        }

        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            for (const auto& neighbor : current->neighbors) {
                if (neighbor->water && !neighbor->ocean) {
                    neighbor->ocean = true;
                    queue.push(neighbor);
                }
            }
        }
    }


private:
   
};

#endif // GRAPH_PASS_WATER_H

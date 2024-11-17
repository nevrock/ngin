#ifndef GRAPH_PASS_MOISTURE_H
#define GRAPH_PASS_MOISTURE_H

#include <snorri_graphs/graph_data.h>
#include <snorri/dict.h>
#include <snorri/log.h>
#include <snorri/noise.h>
#include <vector>
#include <queue>
#include <unordered_set>
#include <random>  // Include for std::random_device and std::mt19937

class GraphPassMoisture {
public:
    GraphPassMoisture() {}

    void executePass(std::vector<std::shared_ptr<GraphCenter>>& centers,
                    std::vector<std::shared_ptr<GraphCorner>>& corners,
                    std::vector<std::shared_ptr<GraphEdge>>& edges, Dict* config) {
        Log::console("graph pass moisture!");

        // Setup random number generator for the corners
        int seed = config->getC<int>("seed", 1238);
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dis(0, corners.size() - 1);

        assignCornerMoisture(corners);
        redistributeMoisture(corners);
        assignCenterMoisture(centers);    
    }
    void redistributeMoisture(std::vector<std::shared_ptr<GraphCorner>>& corners) {
        // Sort corners based on their current moisture levels
        std::sort(corners.begin(), corners.end(), [](const std::shared_ptr<GraphCorner>& a, const std::shared_ptr<GraphCorner>& b) {
            return a->moisture < b->moisture;
        });

        // Redistribute moisture values linearly
        for (size_t i = 0; i < corners.size(); i++) {
            corners[i]->moisture = static_cast<double>(i) / (corners.size() - 1);
        }
    }


    void assignCornerMoisture(std::vector<std::shared_ptr<GraphCorner>>& corners) {
        std::queue<std::shared_ptr<GraphCorner>> queue;

        // Initial moisture setting for corners
        for (auto& q : corners) {
            if ((q->water || q->river > 0) && !q->ocean) {
                q->moisture = q->river > 0 ? std::min(3.0, 0.2 * q->river) : 1.0;
                queue.push(q);
            } else {
                q->moisture = 0.0;
            }
        }

        // Spread moisture to adjacent corners
        while (!queue.empty()) {
            auto q = queue.front();
            queue.pop();

            for (auto& r : q->adjacent) {
                double newMoisture = q->moisture * 0.9;
                if (newMoisture > r->moisture) {
                    r->moisture = newMoisture;
                    queue.push(r);
                }
            }
        }

        // Moisture for corners in the ocean or on the coast
        for (auto& q : corners) {
            if (q->ocean || q->coast) {
                q->moisture = 1.0;
            }
        }
    }
    void assignCenterMoisture(std::vector<std::shared_ptr<GraphCenter>>& centers) {
        // Calculate average moisture for each center
        for (auto& p : centers) {
            double sumMoisture = 0.0;
            for (auto& q : p->corners) {
                if (q->moisture > 1.0) q->moisture = 1.0; // Cap moisture at 1.0
                sumMoisture += q->moisture;
            }
            p->moisture = sumMoisture / p->corners.size();
        }
    }


};

#endif // GRAPH_PASS_MOISTURE_H

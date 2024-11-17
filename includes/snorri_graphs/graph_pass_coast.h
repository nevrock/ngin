#ifndef GRAPH_PASS_COAST_H
#define GRAPH_PASS_COAST_H

#include <snorri_graphs/graph_data.h>
#include <snorri/dict.h>
#include <snorri/log.h>
#include <snorri/noise.h>
#include <vector>
#include <queue>
#include <unordered_set>

class GraphPassCoast {
public:
    GraphPassCoast() {}

    void executePass(std::vector<std::shared_ptr<GraphCenter>>& centers,
                 std::vector<std::shared_ptr<GraphCorner>>& corners,
                 std::vector<std::shared_ptr<GraphEdge>>& edges, Dict* config) {
        Log::console("graph pass coast!");

        for (const auto& center : centers) {
            if (center->border || center->water) {
                continue;
            }
            // land region
            // check neighbours
            bool hasWaterNeighbor = false;
            for (const auto& neighbor : center->neighbors) {
                if (neighbor->ocean) {
                    hasWaterNeighbor = true;
                    break;
                }
            }

            if (hasWaterNeighbor) {
                center->coast = true;
            }
        }

        // Process each corner to determine if it is ocean, coast, or water
        for (auto& q : corners) {
            int numOcean = 0;
            int numLand = 0;

            for (auto& p : q->touches) {
                numOcean += p->ocean ? 1 : 0;
                numLand += !p->water ? 1 : 0;
            }

            q->ocean = (numOcean == q->touches.size());
            q->coast = (numOcean > 0) && (numLand > 0);
            q->water = q->border || ((numLand != q->touches.size()) && !q->coast);
        }
    }


private:
   
};

#endif // GRAPH_PASS_WATER_H

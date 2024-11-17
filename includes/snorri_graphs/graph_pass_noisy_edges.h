#include <cmath>
#include <functional>
#include <memory>
#include <vector>
#include <random>
#include <chrono>

#include <snorri/dict.h>
#include <snorri_graphs/graph_data.h>

class GraphPassNoisyEdges {
public:
    GraphPassNoisyEdges() : noiseLineTradeoff_(0.5f), minLength_(10.0) {
        // Initialize the random number generator with a time-based seed
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        rng_.seed(seed);
    }

    void executePass(std::vector<std::shared_ptr<GraphCenter>>& centers,
                    std::vector<std::shared_ptr<GraphCorner>>& corners,
                    std::vector<std::shared_ptr<GraphEdge>>& edges, Dict* config) {
        Log::console("graph pass noisy edges!");

        for (auto& center : centers) {
            for (auto& edge : center->borders) {
                if (edge->noisy) {
                    continue;
                }
                if (edge->d0 && edge->d1 && edge->v0 && edge->v1) {
                    double f = noiseLineTradeoff_;
                    GraphPoint t = interpolate(edge->v0->point, edge->d0->point, f);
                    GraphPoint q = interpolate(edge->v0->point, edge->d1->point, f);
                    GraphPoint r = interpolate(edge->v1->point, edge->d0->point, f);
                    GraphPoint s = interpolate(edge->v1->point, edge->d1->point, f);

                    updateMinLength(*edge);

                    // Adjust paths to start from each vertex and interpolate towards different points along the edge
                    buildNoisyLineSegments(edge->noisyPoints0, edge->v0->point, t, edge->midpoint, q, minLength_);
                    buildNoisyLineSegments(edge->noisyPoints1, edge->v1->point, s, edge->midpoint, r, minLength_);

                    //if (edge->noisyPoints0.size() > 4) {
                    //    Log::console("graph pass noisy edges, with noisy points 0: " 
                    //        + std::to_string(edge->noisyPoints0.size())
                    //        + ", with distance AC: " + std::to_string(getDistance(edge->v0->point, edge->midpoint))
                    //        + ", with distance BD: " + std::to_string(getDistance(t, q)));
                    //}

                    edge->noisy = true;

                    //Log::console("graph pass noisy edges, with noisy points 0: " 
                    //    + std::to_string(edge->noisyPoints0.size())
                    //    + ", with distance AC: " + std::to_string(getDistance(edge->v0->point, edge->midpoint))
                    //    + ", with distance BD: " + std::to_string(getDistance(t, q)));
                }
            }
        }
    }

private:
    std::mt19937 rng_;  // Random number generator
    float noiseLineTradeoff_;
    float minLength_;

    void updateMinLength(const GraphEdge& edge) {
        if (edge.d0->biome != edge.d1->biome) minLength_ = 0.15;
        if (edge.d0->ocean && edge.d1->ocean) minLength_ = 100;
        if (edge.d0->coast || edge.d1->coast) minLength_ = 0.15;
        if (edge.river) minLength_ = 0.15; // Assuming the presence of a river affects the minimum length
    }

    GraphPoint interpolate(const GraphPoint& p1, const GraphPoint& p2, double f) {
        return {p1.x + f * (p2.x - p1.x), p1.y + f * (p2.y - p1.y)};
    }

    std::vector<GraphPoint> buildNoisyLineSegments(std::vector<GraphPoint>& points, GraphPoint A, GraphPoint B, GraphPoint C, GraphPoint D, int minLength) {
        points.push_back(A);
        int recursionLevel = 0;
        //subdivide(points, A, B, C, D, minLength, recursionLevel);
        points.push_back(C);
        return points;
    }

    void subdivide(std::vector<GraphPoint>& points, GraphPoint A, GraphPoint B, GraphPoint C, GraphPoint D, int minLength, int& recursionLevel) {
        if (recursionLevel >= 3 || getDistance(A, C) < minLength || getDistance(B, D) < minLength) {
            // std::cout << "graph pass noisy edges, length didnt meet criteria for pass." << std::endl;
            return;
        } else {
            // Log::console("graph pass noisy edges subdivide success 1! recursion level - " + std::to_string(recursionLevel));
        }

        std::uniform_real_distribution<double> dist(0.2, 0.8);
        double p = dist(rng_);
        double q = dist(rng_);

        GraphPoint E = interpolate(A, D, p);
        GraphPoint F = interpolate(B, C, p);
        GraphPoint G = interpolate(A, B, q);
        GraphPoint I = interpolate(D, C, q);
        GraphPoint H = interpolate(E, F, q);

        double s = 1.0 - dist(rng_) * 0.8;
        double t = 1.0 - dist(rng_) * 0.8;

        points.push_back(H);

        // Log::console("graph pass noisy edges subdivide success 2! new points E(" + std::to_string(E.x) + ", " + std::to_string(E.y) + 
        //        "), F(" + std::to_string(F.x) + ", " + std::to_string(F.y) + 
        //        "), G(" + std::to_string(G.x) + ", " + std::to_string(G.y) + 
        //        "), H(" + std::to_string(H.x) + ", " + std::to_string(H.y) + 
        //        "), I(" + std::to_string(I.x) + ", " + std::to_string(I.y) + ").");

        recursionLevel++;

        subdivide(points, A, interpolate(G, B, s), H, interpolate(E, D, t), minLength, recursionLevel);
        // Log::console("graph pass noisy edges subdivide success 3! - adding point: " + std::to_string(H.x) + ", " + std::to_string(H.y));
        subdivide(points, H, interpolate(F, C, s), C, interpolate(I, D, t), minLength, recursionLevel);
    }

    double getDistance(const GraphPoint& p1, const GraphPoint& p2) {
        return std::hypot(p2.x - p1.x, p2.y - p1.y);
    }
};

#ifndef GRAPH_GENERATOR_H
#define GRAPH_GENERATOR_H

#include <algorithm> // For std::sort
#include <cmath>     // For atan2 and hypot
#include <vector>
#include <random>
#include <iostream>
#include <glm/glm.hpp>

#include <delaunator/delaunator.hpp>
#include <snorri_graphs/graph_data.h>
#include <snorri_graphs/graph_drawer.h>
 // passes
#include <snorri_graphs/graph_pass_water.h>
#include <snorri_graphs/graph_pass_coast.h>
#include <snorri_graphs/graph_pass_elevation.h>
#include <snorri_graphs/graph_pass_rivers.h>
#include <snorri_graphs/graph_pass_moisture.h>
#include <snorri_graphs/graph_pass_biomes.h>
#include <snorri_graphs/graph_pass_noisy_edges.h>
#include <snorri_graphs/graph_pass_roads.h>
#include <snorri_graphs/graph_pass_towns.h>

#include <snorri/dict.h>

class GraphGenerator {
public:
    GraphGenerator(unsigned int gridSize, double jitter)
        : gridSize_(gridSize), jitter_(jitter) {}

    void loadFromFile(const std::string& filepath) {
        dict_.read(filepath);
        dict_.print();

        gridSize_ = dict_.getC<int>("grid_size", gridSize_);
        jitter_ = dict_.getC<float>("jitter", jitter_);

        generateMap(dict_.getC<int>("seed", 1239));
    }
    void generateMap(int seed) {
        generatePoints(seed);
        triangulatePoints();
        borderCheck();
        executePasses();
    }
    void saveToPNG(const char* filename) {
        GraphDrawer::saveToPNG(filename, 
            gridSize_, dict_.getC<int>("image_size", 512),
            centers, &dict_);
    }
    void executePasses() {
        if (dict_.getC<bool>("is_water", true)) {
            GraphPassWater waterPass;
            waterPass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_coast", true)) {
            GraphPassCoast coastPass;
            coastPass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_elevation", true)) {
            GraphPassElevation elevationPass;
            elevationPass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_rivers", true)) {
            GraphPassRivers riversPass;
            riversPass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_moisture", true)) {
            GraphPassMoisture moisturePass;
            moisturePass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_biomes", true)) {
            GraphPassBiomes biomesPass;
            biomesPass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_roads", true)) {
            GraphPassRoads roadsPass;
            roadsPass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_towns", true)) {
            GraphPassTowns townsPass;
            townsPass.executePass(centers, corners, edges, &dict_);
        }
        if (dict_.getC<bool>("is_noisy_edges", true)) {
            GraphPassNoisyEdges edgesPass;
            edgesPass.executePass(centers, corners, edges, &dict_);
        }
    }


private:
    Dict dict_;
    unsigned int gridSize_;
    double jitter_;
    std::vector<GraphPoint> points_;
    std::vector<std::shared_ptr<GraphCenter>> centers;
    std::vector<std::shared_ptr<GraphCorner>> corners;
    std::vector<std::shared_ptr<GraphEdge>> edges;

    // Generate points with optional jitter
    void generatePoints(int seed) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<> dist(-1.0, 1.0);
        points_.clear();

        for (unsigned int x = 0; x <= gridSize_; x++) {
            for (unsigned int y = 0; y <= gridSize_; y++) {
                double jitterX = x + jitter_ * (dist(gen) - 0.5);
                double jitterY = y + jitter_ * (dist(gen) - 0.5);
                points_.emplace_back(GraphPoint{jitterX, jitterY});
            }
        }

         // Adding boundary points to extend coverage beyond the grid
        double extra = 1.0;  // Extend by one unit beyond the grid size
        for (unsigned int x = 0; x <= gridSize_; x++) {
            // Top and bottom boundary
            points_.push_back({x - extra, -extra}); // Top outside
            points_.push_back({x + extra, gridSize_ + extra}); // Bottom outside
        }
        for (unsigned int y = 0; y <= gridSize_; y++) {
            // Left and right boundary
            points_.push_back({-extra, y - extra}); // Left outside
            points_.push_back({gridSize_ + extra, y + extra}); // Right outside
        }

        // Add corners if not already included
        points_.push_back({-extra, -extra});
        points_.push_back({-extra, gridSize_ + extra});
        points_.push_back({gridSize_ + extra, -extra});
        points_.push_back({gridSize_ + extra, gridSize_ + extra});
    }
    // Perform Delaunay triangulation and prepare for Voronoi construction
    void triangulatePoints() {
        std::vector<double> coords;
        for (const auto& pt : points_) {
            coords.push_back(pt.x);
            coords.push_back(pt.y);
        }
        delaunator::Delaunator d(coords);
        collectDelaunayData(d);
    }
    // Helper function to calculate the centroid of a polygon formed by points
    GraphPoint calculateCentroid(const std::vector<std::shared_ptr<GraphCorner>>& corners) {
        double centroidX = 0, centroidY = 0;
        for (const auto& corner : corners) {
            centroidX += corner->point.x;
            centroidY += corner->point.y;
        }
        size_t count = corners.size();
        return {centroidX / count, centroidY / count};
    }
    // Function to sort corners radially around their centroid
    void sortCorners(std::vector<std::shared_ptr<GraphCorner>>& corners) {
        GraphPoint centroid = calculateCentroid(corners);
        std::sort(corners.begin(), corners.end(), [centroid](const std::shared_ptr<GraphCorner>& a, const std::shared_ptr<GraphCorner>& b) {
            double angleA = std::atan2(a->point.y - centroid.y, a->point.x - centroid.x);
            double angleB = std::atan2(b->point.y - centroid.y, b->point.x - centroid.x);
            return angleA < angleB;
        });
    }
    void borderCheck() {
        for (const auto& center : centers) {
            for (const auto& corner : center->corners) {
                if (corner->point.x < 0 || corner->point.x > gridSize_ ||
                    corner->point.y < 0 || corner->point.y > gridSize_) {
                    corner->border = true;
                    center->border = true;
                    break;
                }
            }
        }
    }
    // Collect data from Delaunay triangulation to construct both Delaunay and Voronoi diagrams
    void collectDelaunayData(const delaunator::Delaunator& delaunay) {
        centers.resize(points_.size());
        corners.clear();
        edges.clear();
        int edgeIndex = 0;
        int cornerIndex = 0;

        // Initialize centers
        for (size_t i = 0; i < points_.size(); ++i) {
            auto center = std::make_shared<GraphCenter>();
            center->index = i;
            center->point = points_[i];
            centers[i] = center;
        }

        // Map to store circumcenters by triangle index to link half-edges later
        std::unordered_map<std::size_t, std::shared_ptr<GraphCorner>> triangleCircumcenters;

        // Calculate corners and link to centers
        for (std::size_t i = 0; i < delaunay.triangles.size(); i += 3) {
            std::size_t i0 = delaunay.triangles[i];
            std::size_t i1 = delaunay.triangles[i + 1];
            std::size_t i2 = delaunay.triangles[i + 2];

            auto circumcenter = calculateCircumcenter(
                points_[i0].x, points_[i0].y,
                points_[i1].x, points_[i1].y,
                points_[i2].x, points_[i2].y
            );

            auto corner = std::make_shared<GraphCorner>();
            corner->point = {circumcenter[0], circumcenter[1]};
            corner->index = cornerIndex++;
            corners.push_back(corner);
            triangleCircumcenters[i / 3] = corner;

            // Assign corner to each center involved in this triangle
            centers[i0]->corners.push_back(corner);
            centers[i1]->corners.push_back(corner);
            centers[i2]->corners.push_back(corner);
            
            corner->touches.push_back(centers[i0]);
            corner->touches.push_back(centers[i1]);
            corner->touches.push_back(centers[i2]);
        }

        for (auto& center : centers) {
            //sortCorners(center->corners);
        }

        // Create half-edge information and connect corners
        for (size_t e = 0; e < delaunay.halfedges.size(); ++e) {
            if (delaunay.halfedges[e] == -1) continue; // Skip edge if it does not have a pair

            size_t e_pair = delaunay.halfedges[e];
            auto edge = std::make_shared<GraphEdge>();
            edge->index = edgeIndex++;

            edge->v0 = triangleCircumcenters[e / 3]; // From current triangle
            edge->v1 = triangleCircumcenters[e_pair / 3]; // To paired triangle

            edge->midpoint = {(edge->v0->point.x + edge->v1->point.x) / 2, (edge->v0->point.y + edge->v1->point.y) / 2};

            edges.push_back(edge);

            // Connect edges to centers and corners
            centers[delaunay.triangles[e_pair]]->borders.push_back(edge);
            centers[delaunay.triangles[e]]->borders.push_back(edge);

            edge->v0->protrudes.push_back(edge);
            edge->v1->protrudes.push_back(edge);

            edge->v0->adjacent.push_back(edge->v1);
            edge->v1->adjacent.push_back(edge->v0);

            edge->d0 = centers[delaunay.triangles[e]];
            edge->d1 = centers[delaunay.triangles[e_pair]];
            
            centers[delaunay.triangles[e]]->neighbors.push_back(centers[delaunay.triangles[e_pair]]);
            centers[delaunay.triangles[e_pair]]->neighbors.push_back(centers[delaunay.triangles[e]]);
        }
    }
    std::array<double, 2> calculateCircumcenter(double x1, double y1, double x2, double y2, double x3, double y3) {
        double D = 2 * (x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2));
        double Ux = ((x1*x1 + y1*y1) * (y2 - y3) + (x2*x2 + y2*y2) * (y3 - y1) + (x3*x3 + y3*y3) * (y1 - y2)) / D;
        double Uy = ((x1*x1 + y1*y1) * (x3 - x2) + (x2*x2 + y2*y2) * (x1 - x3) + (x3*x3 + y3*y3) * (x2 - x1)) / D;
        return {Ux, Uy};
    }
};

#endif // GRAPH_GENERATOR_H

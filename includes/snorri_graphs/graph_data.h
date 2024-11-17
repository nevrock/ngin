
#ifndef GRAPH_DATA_H
#define GRAPH_DATA_H

#include <vector>
#include <string>
#include <memory>

struct GraphPoint {
    double x, y;

    // Calculate Euclidean distance between two points
    double distanceTo(const GraphPoint& other) const {
        return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
    }
};

struct GraphCenter;
struct GraphCorner;
struct GraphEdge;

struct GraphCorner {
    int index;
    GraphPoint point;  // location
    bool ocean;  // ocean
    bool water;  // lake or ocean
    bool coast;  // touches ocean and land polygons
    bool border;  // at the edge of the map
    double elevation;  // 0.0-1.0
    double moisture;  // 0.0-1.0

    std::vector<std::shared_ptr<GraphCenter>> touches;
    std::vector<std::shared_ptr<GraphEdge>> protrudes;
    std::vector<std::shared_ptr<GraphCorner>> adjacent;
    
    int river;  // 0 if no river, or volume of water in river
    std::shared_ptr<GraphCorner> downslope;  // pointer to adjacent corner most downhill
    std::shared_ptr<GraphCorner> watershed;  // pointer to coastal corner, or null
    int watershed_size;
};
struct GraphCenter {
    int index;
    GraphPoint point;  // location
    bool water;  // lake or ocean
    bool ocean;  // ocean
    bool coast;  // land polygon touching an ocean
    bool border;  // at the edge of the map
    std::string biome;  // biome type
    double elevation;  // 0.0-1.0
    double moisture;  // 0.0-1.0

    glm::vec3 color;

    std::vector<std::shared_ptr<GraphCenter>> neighbors;
    std::vector<std::shared_ptr<GraphEdge>> borders;
    std::vector<std::shared_ptr<GraphCorner>> corners;

    // Calculate interpolated elevation at a specific (x, y) location
    double getElevation(double x, double y) const {
        GraphPoint queryPoint = {x, y};
        double totalWeight = 0.0;
        double weightedElevation = 0.0;

        for (const auto& corner : corners) {
            double distance = queryPoint.distanceTo(corner->point);
            if (distance == 0.0) {
                return corner->elevation;  // Directly return the corner's elevation if exactly at the corner
            }
            double weight = 1.0 / distance;  // Inverse distance weighting
            weightedElevation += weight * corner->elevation;
            totalWeight += weight;
        }

        if (totalWeight == 0.0) {
            return 0.0;  // Avoid division by zero
        }

        return weightedElevation / totalWeight;  // Return the weighted average elevation
    }
};
struct GraphEdge {
    int index;
    std::shared_ptr<GraphCenter> d0, d1;  // Delaunay edge
    std::shared_ptr<GraphCorner> v0, v1;  // Voronoi edge
    GraphPoint midpoint;  // halfway between v0,v1
    int river;  // volume of water, or 0
    bool noisy;  // at the edge of the map
    bool road;

    std::vector<GraphPoint> noisyPoints0;  // Noisy points calculated from v0
    std::vector<GraphPoint> noisyPoints1;  // Noisy points calculated from v1

    double getElevation(double x, double y) const {
        GraphPoint queryPoint = {x, y};
        double distanceToV0 = queryPoint.distanceTo(v0->point);
        double distanceToV1 = queryPoint.distanceTo(v1->point);
        double totalDistance = distanceToV0 + distanceToV1;

        if (totalDistance == 0.0) {
            return v0->elevation;  // Avoid division by zero, return v0 elevation if the point is exactly at v0
        }

        double weightV0 = distanceToV1 / totalDistance;
        double weightV1 = distanceToV0 / totalDistance;

        return weightV0 * v0->elevation + weightV1 * v1->elevation;  // Weighted average based on distances
    }
};

#endif  // GRAPH_H

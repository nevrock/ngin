#ifndef GRAPH_DRAWER_H
#define GRAPH_DRAWER_H

#include <vector>
#include <random>
#include <iostream>
#include <glm/glm.hpp>
#include <snorri_graphs/graph_data.h>
#include <snorri/dict.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

class GraphDrawer {
public:
    GraphDrawer() {}

    static void saveToPNG(const char* filename, unsigned int gridSize, unsigned int imageSize, std::vector<std::shared_ptr<GraphCenter>>& centers, Dict* dict) {
        int imageWidth = imageSize;
        int imageHeight = imageSize;
        std::vector<unsigned char> image(imageWidth * imageHeight * 3, 255); // white background

        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<> dist(0, 1);

        Dict defaultBiomeColors;
        Dict* d = dict->get<Dict>("biome_colors", &defaultBiomeColors);

        std::vector<glm::vec3> centerColors;
        for (const auto& center : centers) {
            glm::vec3 color = d->getVec(center->biome, glm::vec3(0.0));
            //color *= float(center->elevation);
            centerColors.push_back(color);
        }

        int i = 0;
        for (auto& center : centers) {
            std::vector<GraphPoint> vertices;
            for (auto& edge : center->borders) {
                for (auto& p : edge->noisyPoints0) {
                    double normalizedX = (p.x / gridSize) * imageSize;
                    double normalizedY = (p.y / gridSize) * imageSize;
                    vertices.push_back({normalizedX, normalizedY});
                }
            }
            
            sortPoints(vertices);
            fillPolygon(vertices, centerColors[i], image, imageWidth, imageHeight, gridSize, center);
            i++;
        }

        for (auto& center : centers) {
            for (auto& edge : center->borders) {
                if (edge->v1->ocean) {
                    continue;
                }
                if (edge->river > 0) {
                    drawRiver(edge, image, imageWidth, imageHeight, edge->river + dict->getC<int>("river_factor", 0), gridSize, dict);
                }
            }
        }

        for (auto& center : centers) {
            for (auto& edge : center->borders) {
                if (edge->road) {
                    drawRoad(edge, image, imageWidth, imageHeight, dict->getC<int>("road_size", 2), gridSize, dict);
                }
            }
        }

        stbi_write_png(filename, imageWidth, imageHeight, 3, image.data(), imageWidth * 3);
    }

private:
    static GraphPoint calculateCentroid(const std::vector<GraphPoint>& points) {
        double centroidX = 0, centroidY = 0;
        for (const auto& point : points) {
            centroidX += point.x;
            centroidY += point.y;
        }
        size_t count = points.size();
        return {centroidX / count, centroidY / count};
    }

    static void sortPoints(std::vector<GraphPoint>& points) {
        GraphPoint centroid = calculateCentroid(points);
        std::sort(points.begin(), points.end(), [centroid](const GraphPoint& a, const GraphPoint& b) {
            double angleA = std::atan2(a.y - centroid.y, a.x - centroid.x);
            double angleB = std::atan2(b.y - centroid.y, b.x - centroid.x);
            return angleA < angleB;
        });
    }

    static void fillPolygon(const std::vector<GraphPoint>& vertices, 
        const glm::vec3& color, std::vector<unsigned char>& image, 
        unsigned int imageWidth, unsigned int imageHeight, unsigned int gridSize, 
        const std::shared_ptr<GraphCenter>& center) {
        if (vertices.empty()) return;

        GraphPoint centroid = calculateCentroid(vertices);
        size_t n = vertices.size();

        for (size_t i = 0; i < n; ++i) {
            std::vector<GraphPoint> triangle = {centroid, vertices[i], vertices[(i + 1) % n]};
            fillTriangle(triangle, color, image, imageWidth, imageHeight, gridSize, center);
        }
    }
    static void fillTriangle(const std::vector<GraphPoint>& triangle, 
        const glm::vec3& color, std::vector<unsigned char>& image, 
        unsigned int imageWidth, unsigned int imageHeight, unsigned int gridSize, 
        const std::shared_ptr<GraphCenter>& center) {
        if (triangle.size() < 3) return; // Safety check

        auto minmaxX = std::minmax_element(triangle.begin(), triangle.end(), [](const GraphPoint& a, const GraphPoint& b) { return a.x < b.x; });
        auto minmaxY = std::minmax_element(triangle.begin(), triangle.end(), [](const GraphPoint& a, const GraphPoint& b) { return a.y < b.y; });

        int minX = std::max(0, static_cast<int>(floor(minmaxX.first->x)));
        int maxX = std::min(static_cast<int>(imageWidth - 1), static_cast<int>(ceil(minmaxX.second->x)));
        int minY = std::max(0, static_cast<int>(floor(minmaxY.first->y)));
        int maxY = std::min(static_cast<int>(imageHeight - 1), static_cast<int>(ceil(minmaxY.second->y)));

        auto edgeFunction = [](const GraphPoint& a, const GraphPoint& b, const GraphPoint& c) {
            return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
        };

        const GraphPoint& v0 = triangle[0];
        const GraphPoint& v1 = triangle[1];
        const GraphPoint& v2 = triangle[2];

        float area = edgeFunction(v0, v1, v2);
        bool hasPositiveArea = area > 0;

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                GraphPoint pt = {static_cast<double>(x), static_cast<double>(y)};
                float w0 = edgeFunction(v1, v2, pt);
                float w1 = edgeFunction(v2, v0, pt);
                float w2 = edgeFunction(v0, v1, pt);

                if ((w0 == 0 || w1 == 0 || w2 == 0) || (hasPositiveArea ? (w0 > 0 && w1 > 0 && w2 > 0) : (w0 < 0 && w1 < 0 && w2 < 0))) {
                    double gridX = pt.x * gridSize / imageWidth;
                    double gridY = pt.y * gridSize / imageHeight;
                    double elevation = center->getElevation(gridX, gridY);
                    
                    int index = (y * imageWidth + x) * 3;
                    std::cout << "Setting pixel at " << x << ", " << y << ": " 
                                << "(" << color.r * elevation << ", " 
                                << color.g * elevation << ", " 
                                << color.b * elevation << ")" << std::endl;
                    image[index]     = static_cast<unsigned char>(color.r * elevation);
                    image[index + 1] = static_cast<unsigned char>(color.g * elevation);
                    image[index + 2] = static_cast<unsigned char>(color.b * elevation);
                }
            }
        }
    }
    static void drawRiver(const std::shared_ptr<GraphEdge>& edge, std::vector<unsigned char>& image, unsigned int imageWidth, unsigned int imageHeight, int riverSize, int gridSize, Dict* dict) {
        int x0 = static_cast<int>((edge->v0->point.x / gridSize) * imageWidth);
        int y0 = static_cast<int>((edge->v0->point.y / gridSize) * imageHeight);
        int x1 = static_cast<int>((edge->v1->point.x / gridSize) * imageWidth);
        int y1 = static_cast<int>((edge->v1->point.y / gridSize) * imageHeight);

        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        
        glm::vec3 riverColor = dict->getVec("river_color", glm::vec3(0.0));

        while (true) {
            double gridX0 = x0 * gridSize / imageWidth;
            double gridY0 = y0 * gridSize / imageHeight;
            double elevation = edge->getElevation(gridX0, gridY0);

            for (int i = -riverSize; i <= riverSize; ++i) {
                for (int j = -riverSize; j <= riverSize; ++j) {
                    int nx = x0 + i;
                    int ny = y0 + j;
                    if (nx >= 0 && nx < imageWidth && ny >= 0 && ny < imageHeight) {
                        int index = (ny * imageWidth + nx) * 3;
                        image[index] = static_cast<unsigned char>(riverColor.r*elevation);
                        image[index + 1] = static_cast<unsigned char>(riverColor.g*elevation);
                        image[index + 2] = static_cast<unsigned char>(riverColor.b*elevation);
                    }
                }
            }

            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
    static void drawRoad(const std::shared_ptr<GraphEdge>& edge, std::vector<unsigned char>& image, 
        unsigned int imageWidth, unsigned int imageHeight, 
        int roadSize, int gridSize, Dict* dict) {
        int x0 = static_cast<int>((edge->d0->point.x / gridSize) * imageWidth);
        int y0 = static_cast<int>((edge->d0->point.y / gridSize) * imageHeight);
        int x1 = static_cast<int>((edge->d1->point.x / gridSize) * imageWidth);
        int y1 = static_cast<int>((edge->d1->point.y / gridSize) * imageHeight);

        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        
        glm::vec3 roadColor = dict->getVec("road_color", glm::vec3(0.0));

        while (true) {
            double gridX0 = x0 * gridSize / imageWidth;
            double gridY0 = y0 * gridSize / imageHeight;
            double elevation = edge->getElevation(gridX0, gridY0);

            for (int i = -roadSize; i <= roadSize; ++i) {
                for (int j = -roadSize; j <= roadSize; ++j) {
                    int nx = x0 + i;
                    int ny = y0 + j;
                    if (nx >= 0 && nx < imageWidth && ny >= 0 && ny < imageHeight) {
                        int index = (ny * imageWidth + nx) * 3;
                        image[index] = static_cast<unsigned char>(roadColor.r * elevation);
                        image[index + 1] = static_cast<unsigned char>(roadColor.g * elevation);
                        image[index + 2] = static_cast<unsigned char>(roadColor.b * elevation);
                    }
                }
            }

            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
};

#endif // GRAPH_DRAWER_H

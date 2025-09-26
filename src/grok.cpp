#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>
#include "OneLoneCoder/olcPixelGameEngine.h"

// Assuming olc::vf2d has float x, y and basic operations
// struct olc::vf2d {
//     float x, y;
//     olc::vf2d(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) {}
//     olc::vf2d operator+(const olc::vf2d& other) const { return {x + other.x, y + other.y}; }
//     olc::vf2d operator-(const olc::vf2d& other) const { return {x - other.x, y - other.y}; }
//     olc::vf2d operator*(float s) const { return {x * s, y * s}; }
// };

// // Assuming olc::vi2d for integer coordinates
// struct olc::vi2d {
//     int x, y;
//     olc::vi2d(int _x = 0, int _y = 0) : x(_x), y(_y) {}
// };

enum DIRECTIONS {
    DIR_UNDEFINED,
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
    DIR_HORIZONTAL,
    DIR_VERTICAL,
    DIR_START,
    DIR_END
};

struct sPath {
    std::vector<float> nodes;
    olc::vf2d origin;
    bool InDirection(int nDir, olc::vf2d vfPos, int nNode) const; // Provided
    olc::vf2d GetStartAbs(int nNode) const; // Provided
    olc::vf2d GetEndAbs(int nNode) const; // Provided
};

// Compute vertices from sPath
std::vector<olc::vf2d> GetVertices(const sPath& path) {
    size_t n = path.nodes.size();
    std::vector<olc::vf2d> verts(n);
    olc::vf2d curr = path.origin;
    verts[0] = curr;
    bool horz = true;
    for (size_t i = 0; i < n - 1; ++i) {
        if (horz) curr.x += path.nodes[i];
        else curr.y += path.nodes[i];
        verts[i + 1] = curr;
        horz = !horz;
    }
    return verts;
}

// Find reflex vertices using InDirection
std::vector<std::pair<size_t, int>> FindReflex(const sPath& path, const std::vector<olc::vf2d>& verts) {
    size_t n = verts.size();
    std::vector<std::pair<size_t, int>> reflex; // (vertex index, inward direction)
    bool horz = true;
    for (size_t i = 0; i < n - 1; ++i) {
        olc::vf2d v = verts[i];
        int nNode = static_cast<int>(i);
        // At vertex i, check directions perpendicular to current edge
        if (horz) {
            if (path.InDirection(DIR_UP, v, nNode)) reflex.emplace_back(i, DIR_UP);
            if (path.InDirection(DIR_DOWN, v, nNode)) reflex.emplace_back(i, DIR_DOWN);
        } else {
            if (path.InDirection(DIR_LEFT, v, nNode)) reflex.emplace_back(i, DIR_LEFT);
            if (path.InDirection(DIR_RIGHT, v, nNode)) reflex.emplace_back(i, DIR_RIGHT);
        }
        horz = !horz;
    }
    return reflex;
}

// Struct for hit info
struct HitInfo {
    size_t edge;
    float u;
    float t;
};

// Recursive decomposition
void ChordDecompose(const sPath& path, const std::vector<olc::vf2d>& poly, std::vector<std::pair<olc::vi2d, olc::vi2d>>& rectangles) {
    size_t n = poly.size();
    if (n < 4) return; // Invalid

    // Check if polygon is a rectangle
    bool is_rectangle = true;
    bool horz = true;
    for (size_t i = 0; i < n - 1; ++i) {
        olc::vf2d curr = poly[i];
        olc::vf2d next = poly[(i + 1) % n];
        if (horz && std::abs(curr.y - next.y) > 1e-6f) { is_rectangle = false; break; }
        if (!horz && std::abs(curr.x - next.x) > 1e-6f) { is_rectangle = false; break; }
        horz = !horz;
    }
    if (is_rectangle) {
        float min_xf = poly[0].x, max_xf = poly[0].x, min_yf = poly[0].y, max_yf = poly[0].y;
        for (const auto& v : poly) {
            min_xf = std::min(min_xf, v.x);
            max_xf = std::max(max_xf, v.x);
            min_yf = std::min(min_yf, v.y);
            max_yf = std::max(max_yf, v.y);
        }
        // Round to nearest integer; adjust rounding strategy during testing
        int min_x = static_cast<int>(std::round(min_xf));
        int max_x = static_cast<int>(std::round(max_xf));
        int min_y = static_cast<int>(std::round(min_yf));
        int max_y = static_cast<int>(std::round(max_yf));
        rectangles.push_back({{min_x, min_y}, {max_x, max_y}}); // Top-left, bottom-right
        return;
    }

    // Find reflex vertices with inward directions
    auto reflex = FindReflex(path, poly);
    if (reflex.empty()) return; // Shouldn't happen for non-rectangle

    // Pick first reflex vertex and its inward direction
    size_t i = reflex[0].first;
    int dir = reflex[0].second;
    olc::vf2d start = poly[i];
    olc::vf2d ray_dir;
    if (dir == DIR_UP) ray_dir = {0, -1};
    else if (dir == DIR_DOWN) ray_dir = {0, 1};
    else if (dir == DIR_LEFT) ray_dir = {-1, 0};
    else if (dir == DIR_RIGHT) ray_dir = {1, 0};

    // Find intersections
    std::vector<HitInfo> hits;
    for (size_t j = 0; j < n; ++j) {
        size_t next_j = (j + 1) % n;
        olc::vf2d a = poly[j];
        olc::vf2d b = poly[next_j];
        olc::vf2d e = b - a;
        float den = ray_dir.x * e.y - ray_dir.y * e.x;
        if (std::abs(den) < 1e-6f) continue; // Parallel
        float tx = (e.y * (a.x - start.x) - e.x * (a.y - start.y)) / den;
        float tu = (ray_dir.y * (a.x - start.x) - ray_dir.x * (a.y - start.y)) / den;
        if (tx > 1e-6f && tu >= 0.0f && tu <= 1.0f) {
            hits.push_back({j, tu, tx});
        }
    }

    if (hits.empty()) return; // No valid hit

    // Find closest hit
    auto min_hit = *std::min_element(hits.begin(), hits.end(), [](const HitInfo& a, const HitInfo& b) {
        return a.t < b.t;
    });
    size_t hit_edge = min_hit.edge;
    float u = min_hit.u;
    olc::vf2d h = poly[hit_edge] + (poly[(hit_edge + 1) % n] - poly[hit_edge]) * u;

    // Create subpoly1: h -> (hit_edge+1) -> ... -> i
    std::vector<olc::vf2d> sub1;
    sub1.push_back(h);
    size_t j = (hit_edge + 1) % n;
    while (j != i) {
        sub1.push_back(poly[j]);
        j = (j + 1) % n;
    }
    sub1.push_back(poly[i]);

    // Create subpoly2: h -> hit_edge -> (hit_edge-1) -> ... -> i
    std::vector<olc::vf2d> sub2;
    sub2.push_back(h);
    j = hit_edge;
    while (j != i) {
        sub2.push_back(poly[j]);
        j = (j + n - 1) % n;
    }
    sub2.push_back(poly[i]);

    // Recur on sub-polygons
    ChordDecompose(path, sub1, rectangles);
    ChordDecompose(path, sub2, rectangles);
}

// Main function to decompose polygon into rectangles
void DecomposePolygon(const sPath& path, std::vector<std::pair<olc::vi2d, olc::vi2d>>& rectangles) {
    std::vector<olc::vf2d> verts = GetVertices(path);
    ChordDecompose(path, verts, rectangles);
}
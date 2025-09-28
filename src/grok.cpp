#include "OneLoneCoder/olcPixelGameEngine.h"
#include "main.cpp" // remove for the love of god


// Assuming olc::vf2d is defined as in your struct with x, y and basic operations
// struct olc::vf2d {
//     float x, y;
//     olc::vf2d(float _x, float _y) : x(_x), y(_y) {}
//     olc::vf2d operator+(const olc::vf2d& other) const { return {x + other.x, y + other.y}; }
// };

// Assuming sPath is as provided with nodes, IsTargetInside, etc.

// Function to decompose polygon into rectangles with integer coordinates
void DecomposePolygon(const sPath& path, std::vector<std::pair<olc::vi2d, olc::vi2d>>& rectangles, 
                     int width = 290, int height = 190, int min_area = 100) {
    // Define Rectangle struct for recursion
    struct Rect {
        int x1, y1; // Top-left (min x, min y)
        int x2, y2; // Bottom-right (max x, max y)
    };

    // Recursive function to process a rectangle
    auto Decompose = [&](auto& self, const Rect& rect) -> void {
        // Check area threshold to stop recursion
        int area = (rect.x2 - rect.x1) * (rect.y2 - rect.y1);
        if (area < min_area) {
            // If any corner inside, include the rectangle (conservative to avoid gaps)
            olc::vf2d corners[4] = {
                {float(rect.x1), float(rect.y1)},
                {float(rect.x2), float(rect.y1)},
                {float(rect.x1), float(rect.y2)},
                {float(rect.x2), float(rect.y2)}
            };
            for (const auto& corner : corners) {
                if (path.IsTargetInside(corner)) {
                    rectangles.emplace_back(olc::vi2d(rect.x1, rect.y1), olc::vi2d(rect.x2, rect.y2));
                    break;
                }
            }
            return;
        }

        // Check if all four corners are inside the polygon
        olc::vf2d corners[4] = {
            {float(rect.x1), float(rect.y1)},
            {float(rect.x2), float(rect.y1)},
            {float(rect.x1), float(rect.y2)},
            {float(rect.x2), float(rect.y2)}
        };
        int inside_count = 0;
        for (const auto& corner : corners) {
            if (path.IsTargetInside(corner)) {
                inside_count++;
            }
        }

        if (inside_count == 4) {
            // Fully inside: add rectangle
            rectangles.emplace_back(olc::vi2d(rect.x1, rect.y1), olc::vi2d(rect.x2, rect.y2));
            return;
        }
        if (inside_count == 0) {
            // Fully outside: discard
            return;
        }

        // Partially inside: subdivide into 4 quadrants
        int mid_x = (rect.x1 + rect.x2) / 2;
        int mid_y = (rect.y1 + rect.y2) / 2;

        // Ensure mid points are integers; handle odd dimensions
        if ((rect.x2 - rect.x1) % 2 != 0) mid_x += 1; // Bias to avoid 0-width
        if ((rect.y2 - rect.y1) % 2 != 0) mid_y += 1; // Bias to avoid 0-height

        // Subdivide into four rectangles
        self(self, {rect.x1, rect.y1, mid_x, mid_y}); // Top-left
        self(self, {mid_x, rect.y1, rect.x2, mid_y}); // Top-right
        self(self, {rect.x1, mid_y, mid_x, rect.y2}); // Bottom-left
        self(self, {mid_x, mid_y, rect.x2, rect.y2}); // Bottom-right
    };

    // Start with the bounding rectangle
    Rect initial_rect = {0, 0, width, height};
    Decompose(Decompose, initial_rect);
}
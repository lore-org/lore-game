#include <raylib-cpp.hpp>
#include <fmt/base.h>
#include <fmt/format.h>

int main() {
    raylib::InitWindow(720, 480);
    SetTargetFPS(60);

    int hue = 0;

    while (!WindowShouldClose()) {

        if (hue >= 360) hue = 0;

        BeginDrawing();

        ClearBackground(ColorFromHSV(++hue, 1, 1));
        raylib::DrawText(fmt::format("{} FPS", GetFPS()), 5, 5, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
}
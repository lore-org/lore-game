#include <raylib-cpp.hpp>
#include <fmt/base.h>

int main() {
    raylib::InitWindow(720, 480);
    SetTargetFPS(60);

    int hue = 0;

    while (!WindowShouldClose()) {

        if (hue >= 360) hue = 0;

        BeginDrawing();

        ClearBackground(ColorFromHSV(++hue, 1, 1));
        DrawFPS(0, 0);

        fmt::println("h: {}deg", hue);

        EndDrawing();
    }

    CloseWindow();
}
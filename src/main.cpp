#include "raylib.h"
#include <raylib-cpp.hpp>
#include <fmt/base.h>
#include <fmt/format.h>

#include "Data.hpp"
#include "Sprite.hpp"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    raylib::InitWindow(720, 480);

    auto spr = Sprite::createFromFile("resources/kitty.png");

    int hue = 0;

    // TODO - run scheduler in seperate thread from frame updates

    while (!WindowShouldClose()) {
        if (hue >= 360) hue %= 360;

        BeginDrawing();

        ClearBackground(ColorFromHSV(++hue, 1, 1));
        raylib::DrawText(fmt::format("{} FPS", GetFPS()), 5, 5, 20, BLACK);

        // TODO - SceneManager to run draw calls
        spr->setRotation(spr->getRotation() + 1.f);
        spr->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2);
        spr->draw();

        EndDrawing();
    }

    spr->release();
    CloseWindow();
}
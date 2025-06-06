#include <raylib-cpp.hpp>
#include <fmt/base.h>
#include <fmt/format.h>

#include "Data.hpp"
#include "Sprite.hpp"

int main() {
    raylib::InitWindow(720, 480);
    SetTargetFPS(240);

    auto spr = Sprite::createFromFile("resources/kitty.png");
    spr->setPosition(Point(GetScreenWidth(), GetScreenHeight()) / 2);

    int hue = 0;

    while (!WindowShouldClose()) {
        if (hue >= 360) hue %= 360;

        BeginDrawing();

        ClearBackground(ColorFromHSV(++hue, 1, 1));
        raylib::DrawText(fmt::format("{} FPS", GetFPS()), 5, 5, 20, BLACK);

        // TODO - SceneManager to run draw calls
        spr->setRotation(spr->getRotation() + 1.f);
        spr->draw();

        EndDrawing();
    }

    spr->release();
    CloseWindow();
}
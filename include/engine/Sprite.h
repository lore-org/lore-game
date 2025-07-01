#pragma once
#include "Default.h" // IWYU pragma: keep

#include <memory>

#include "ColorNode.h"

class Sprite : public ColorNode {
public:
    virtual ~Sprite();

    virtual bool init(SDL_Texture* texture);

    static std::shared_ptr<Sprite> create();
    static std::shared_ptr<Sprite> createFromFile(std::string filename);
    static std::shared_ptr<Sprite> createFromSurface(SDL_Surface* surface);

    void setTexture(SDL_Texture* texture);
    auto getTexture();

    virtual void draw(const double dt) override;

protected:
    Sprite();

    SDL_Texture* m_texture;
};
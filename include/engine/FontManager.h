#pragma once

#include <cstdlib>
#include <engine/Object.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <simdutf.h>

class FontManager : public Object {
public:
    ~FontManager();

    static std::shared_ptr<FontManager> sharedManager();

    FT_Library getFTLibrary();

    struct FontFace {
    public:
        FT_Face ftFontFace;

        void setFontPoint(float point);
        float getFontPoint() { return m_point; }

        void loadGlyph(char codepoint);
        void loadGlyph(char16_t codepoint);
        void loadGlyph(char32_t codepoint);

        bool operator==(FontFace second) {
            return this->ftFontFace == second.ftFontFace;
        }

    protected:
        float m_point;
    };

    struct Bitmap {
        char* bitmap;
        // width and height
        int bitmapSize;
        short bitmapChannels;

        static Bitmap* create(int size = 1024, short channels = 1);

        // Resize from the top-left corner
        void resize(int size);
        // Get pointer to pixel at given coordinates
        char* getPixel(int x, int y);
    };

    struct Glyph {

    };

    FontFace* getFontFace(std::string file);
    FontFace* getFontFace(FT_Face fontFace);
    std::string getFontFile(FontFace* fontFace);
    FontFace* createFontFace(std::string file);
    FontFace* getOrCreateFontFace(std::string file);

protected:
    FontManager() = default;

    FT_Library m_FTLibrary = nullptr;
    std::unordered_map<std::string, FontFace> m_fontFaceMap;
    std::unordered_map<FT_Face, std::vector<Glyph>> m_renderedGlyphs;

private:
    static std::shared_ptr<FontManager> m_instance;
};
#pragma once

#include <cstdlib>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <simdutf.h>
#include <rectpack2D/finders_interface.h>

#include <engine/Object.h>

// Disallow texture flipping and use default allocator
using empty_spaces = rectpack2D::empty_spaces<false>;
using rect_t = rectpack2D::output_rect_t<empty_spaces>;

class FontManager : public Object {
public:
    ~FontManager();

    static std::shared_ptr<FontManager> sharedManager();

    FT_Library getFTLibrary();

    struct Bitmap {
    public:
        Bitmap(int size = 1024, short channels = 1);
        ~Bitmap();

        char* m_bitmap;
        // width and height
        int m_bitmapSize;
        short m_bitmapChannels;

        static Bitmap* create(int size = 1024, short channels = 1);

        // Resize from the top-left corner
        void resize(int size);
        // Get pointer to pixel at given coordinates
        char* getPixel(int x, int y);
        void drawPixels(rect_t dimensions, char* data);
    };

    struct Atlas : public Bitmap {
    public:
        Atlas(int size = 1024, short channels = 1);
        ~Atlas() = default;

        rect_t insertRect(int width, int height);
        // Resize from the top-left corner
        void resize(int size);

    protected:
        std::vector<rect_t> m_placedRects;
        empty_spaces m_packer;
    };

    struct Glyph {
        FT_Face ftFontFace;
        Atlas* glyphAtlas;

        FT_ULong codepoint;
        FT_UInt glyphIndex;

        int atlasX, atlasY; // Atlas offset
        int offsetX, offsetY; // Top-left offset from cursor
        int width, height; // Dimensions
        int advanceX; // Next cursor's offset
    };

    struct FontFace {
    public:
        FontFace(FT_Face font, float point = 14);
        ~FontFace();

        FT_Face m_ftFontFace;
        Atlas* m_glyphAtlas;

        bool operator==(FontFace second);

        void setFontPoint(float point);
        float getFontPoint() { return m_point; }

        Glyph* loadGlyph(char codepoint);
        Glyph* loadGlyph(char16_t codepoint);
        Glyph* loadGlyph(char32_t codepoint);

        std::vector<Glyph*> loadString(std::string string);
        std::vector<Glyph*> loadString(std::u32string string);

    protected:
        float m_point;
        std::unordered_map<char32_t, Glyph*> m_renderedGlyphs;
    };

    FontFace* getFontFace(std::string file);
    FontFace* getFontFace(FT_Face fontFace);
    std::string getFontFile(FontFace* fontFace);
    FontFace* createFontFace(std::string file);
    FontFace* getOrCreateFontFace(std::string file);

protected:
    FontManager() = default;

    FT_Library m_FTLibrary = nullptr;
    std::unordered_map<std::string, FontFace*> m_fontFaceMap;

private:
    static std::shared_ptr<FontManager> m_instance;
};
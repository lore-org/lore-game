#pragma once

#include <cstdlib>

#include <ft2build.h>
#include <string>
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

        char* m_bitmap = nullptr;
        // width and height
        int m_bitmapSize;
        short m_bitmapChannels;

        static Bitmap* create(int size = 1024, short channels = 1);

        // Resize from the top-left corner
        void resize(int size);
        // Get a pointer to a pixel from a given buffer at the given coordinates
        template <typename T>
        static T* getPixel(int x, int y, T* buffer, int width, int height, short stride);
        // Get a pointer to a pixel from the current bitmap at the given coordinates
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
        FT_Face ftFontFace = nullptr;
        Atlas* glyphAtlas = nullptr;

        FT_ULong codepoint;
        FT_UInt glyphIndex;

        int atlasX, atlasY; // Atlas offset
        float offsetX, offsetY; // Top-left offset from cursor
        int width, height; // Dimensions
        float advanceX; // Next cursor's offset
    };

    struct FontFace {
    public:
        FontFace(FT_Face font, float point = 14);
        ~FontFace();

        FT_Face m_ftFontFace = nullptr;
        Atlas* m_glyphAtlas = nullptr;

        // Vertical offset to the next baseline of a new line
        float m_lineSpacing;
        // Global vertical ascension from the baseline (Not the maximum ascent)
        float m_globalAscender;
        // Global vertical descension from the baseline (Not the maximuim descent)
        float m_globalDescender;
        // Vertical offset from the baseline of the underline.
        float m_underlineOffset;
        // Vertical thickness of the underline
        float m_underlineThickness;

        bool operator==(FontFace second);

        void setFontPoint(float point);
        float getFontPoint() { return m_point; }

        Glyph* loadGlyph(char codepoint);
        Glyph* loadGlyph(char16_t codepoint);
        Glyph* loadGlyph(char32_t codepoint);

        std::vector<Glyph*> loadString(std::string string);
        std::vector<Glyph*> loadString(std::u16string string);
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
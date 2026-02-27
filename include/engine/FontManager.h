#pragma once

#include <engine/utils.h>
#include <engine/Object.h>

#include <ft2build.h>
#include FT_FREETYPE_H

class FontManager : public Object {
public:
    ~FontManager();

    static std::shared_ptr<FontManager> sharedManager();

    FT_Library getFTLibrary();

    // Mapped to <file, point>
    struct FontFace {
        FT_Face fontFace;
        float point;
    };

    FT_Face getFontFace(std::string file);
    std::string getFontFile(FT_Face fontFace);
    FT_Face createFontFace(std::string file);
    FT_Face getOrCreateFontFace(std::string file);

    static void setFontPoint(FT_Face font, float point);
    static float getFontPoint(FT_Face font);

protected:
    FontManager() = default;

    FT_Library m_FTLibrary = nullptr;
    std::unordered_map<std::string, FontFace> m_fontFaceMap;

private:
    static std::shared_ptr<FontManager> m_instance;
};
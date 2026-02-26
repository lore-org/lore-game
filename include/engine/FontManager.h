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
    using FontFaceDict = std::pair<std::string, float>;

    FT_Face getFontFace(std::string file, float point);
    FontFaceDict getFontDict(FT_Face fontFace);
    FT_Face createFontFace(std::string file, float point);
    FT_Face getOrCreateFontFace(std::string file, float point);

    static void setFontPoint(FT_Face font, float point);

protected:
    FontManager();

    FT_Library m_FTLibrary;
    std::unordered_map<FontFaceDict, FT_Face, utils::hash_pair> m_fontFaceMap;

private:
    static std::shared_ptr<FontManager> m_instance = nullptr;
};
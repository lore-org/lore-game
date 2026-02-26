#include "freetype/freetype.h"
#include <engine/FontManager.h>

#include <engine/Engine.h>
#include <engine/utils.h>

std::shared_ptr<FontManager> FontManager::m_instance;

FontManager::~FontManager() {
    for (auto& storedFontFace : m_fontFaceMap)
        FT_Done_Face(storedFontFace.second); // TODO - check for errors

    FT_Done_FreeType(m_FTLibrary); // TODO - check for errors
}

std::shared_ptr<FontManager> FontManager::sharedManager() {
    if (!m_instance) m_instance = utils::protected_make_shared<FontManager>();
    return m_instance;
}

FT_Library FontManager::getFTLibrary() {
    if (!m_FTLibrary) {
        if (auto e = FT_Init_FreeType(&m_FTLibrary)) {
            LogError("Could not initialise FreeType");
            LogDebug(FT_Error_String(e));
        }
    }
    return m_FTLibrary;
}

FT_Face FontManager::getFontFace(std::string file, float point) {
    FontFaceDict key(file, point);
    if (m_fontFaceMap.contains(key)) return m_fontFaceMap.at(key);

    return nullptr;
}

FontManager::FontFaceDict FontManager::getFontDict(FT_Face fontFace) {
    for (auto& storedFontFace : m_fontFaceMap) {
        if (storedFontFace.second == fontFace) return storedFontFace.first;
    }

    return { };
}

FT_Face FontManager::createFontFace(std::string file, float point) {
    FT_Face fontFace;

    if (auto e = FT_New_Face(m_FTLibrary, file.c_str(), 0, &fontFace)) {
        LogError(fmt::format("Could not create font face (file={})", file));
        LogDebug(FT_Error_String(e));
        return nullptr;
    }
    
    this->setFontPoint(fontFace, point);

    FontFaceDict key(file, point);
    if (m_fontFaceMap.contains(key))
        FT_Done_Face(m_fontFaceMap[key]); // Delete existing face (TODO - check for errors)
    m_fontFaceMap[key] = fontFace;

    return fontFace;
}

FT_Face FontManager::getOrCreateFontFace(std::string file, float point) {
    if (auto fontFace = this->getFontFace(file, point)) return fontFace;
    else return this->createFontFace(file, point);
}

void FontManager::setFontPoint(FT_Face font, float point) {
    auto engine = Engine::sharedInstance();
    auto dpi = engine->getMonitorDPI(engine->getCurrentMonitor());
    if (auto e = FT_Set_Char_Size(font, 0, point * 64, dpi.width, dpi.height)) {
        LogError(fmt::format("Could not resize font face (point={})", point));
        LogDebug(FT_Error_String(e));
    }
}
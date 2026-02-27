#include "freetype/freetype.h"
#include <engine/FontManager.h>

#include <engine/Engine.h>
#include <engine/utils.h>

#define log_freetype_error() do { if (auto s = FT_Error_String(e)) LogDebug(s); else LogDebug("FT_Error_String returned NULL"); } while (0)


std::shared_ptr<FontManager> FontManager::m_instance;

FontManager::~FontManager() {
    for (auto& storedFontFace : m_fontFaceMap)
        FT_Done_Face(storedFontFace.second.fontFace); // TODO - check for errors

    FT_Done_FreeType(this->getFTLibrary()); // TODO - check for errors
}

std::shared_ptr<FontManager> FontManager::sharedManager() {
    if (!m_instance) m_instance = utils::protected_make_shared<FontManager>();
    return m_instance;
}

FT_Library FontManager::getFTLibrary() {
    if (!m_FTLibrary) {
        if (auto e = FT_Init_FreeType(&m_FTLibrary)) {
            LogError("Could not initialise FreeType");
            log_freetype_error();
        }
    }
    return m_FTLibrary;
}

FT_Face FontManager::getFontFace(std::string file) {
    if (m_fontFaceMap.contains(file)) return m_fontFaceMap[file].fontFace;

    return nullptr;
}

std::string FontManager::getFontFile(FT_Face fontFace) {
    for (auto& storedFontFace : m_fontFaceMap) {
        if (storedFontFace.second.fontFace == fontFace) return storedFontFace.first;
    }

    return { };
}

FT_Face FontManager::createFontFace(std::string file) {
    FT_Face fontFace;

    if (auto e = FT_New_Face(this->getFTLibrary(), file.c_str(), 0, &fontFace)) {
        LogError(fmt::format("Could not create font face (file={})", file));
        log_freetype_error();
        return nullptr;
    }
    
    this->setFontPoint(fontFace, 12);

    if (m_fontFaceMap.contains(file))
        FT_Done_Face(m_fontFaceMap[file].fontFace); // Delete existing face (TODO - check for errors)
    m_fontFaceMap[file].fontFace = fontFace;

    return fontFace;
}

FT_Face FontManager::getOrCreateFontFace(std::string file) {
    if (auto fontFace = this->getFontFace(file)) return fontFace;
    else return this->createFontFace(file);
}

void FontManager::setFontPoint(FT_Face font, float point) {
    auto engine = Engine::sharedInstance();
    auto dpi = engine->getMonitorDPI(engine->getCurrentMonitor());
    if (auto e = FT_Set_Char_Size(font, 0, point * 64, dpi.width, dpi.height)) {
        LogError(fmt::format("Could not resize font face (point={})", point));
        LogDebug(FT_Error_String(e) || "No error description given.");
    }
}

float FontManager::getFontPoint(FT_Face font) {
    auto fontManager = FontManager::sharedManager();
    auto fontFile = fontManager->getFontFile(font);
    if (!fontManager->m_fontFaceMap.contains(fontFile)) return 0;
    return fontManager->m_fontFaceMap[fontFile].point;
}
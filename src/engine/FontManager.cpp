#include <engine/FontManager.h>

#include <engine/utils.h>

std::shared_ptr<FontManager> FontManager::m_instance;

FontManager::~FontManager() {
    for (auto& storedFontFace : m_fontFaceMap)
        FT_Done_Face(storedFontFace.second);

    FT_Done_FreeType(m_FTLibrary);
}

std::shared_ptr<FontManager> FontManager::sharedManager() {
    if (!m_instance) m_instance = utils::protected_make_shared<FontManager>();
    return m_instance;
}

FT_Library FontManager::getFTLibrary() {
    if (!m_FTLibrary) FT_Init_FreeType(&m_FTLibrary);
    if (!m_FTLibrary) LogError("Could not initialise FreeType");
    return m_FTLibrary;
}

FT_Face FontManager::getFontFace(std::string file, float point) {
    FontFaceDict key(file, point);
    if (m_fontFaceMap.contains(key)) return m_fontFaceMap.at(key);

    return nullptr;
}

FontManager::FontFaceDict FontManager::getFontFace(FT_Face fontFace) {
    for (auto& storedFontFace : m_fontFaceMap) {
        if (storedFontFace.second == fontFace) return storedFontFace.first;
    }
}

FT_Face FontManager::createFontFace(std::string file, float point) {
    FT_Face fontFace;

    if (FT_New_Face(m_FTLibrary, file.c_str(), 0, &fontFace))
        LogError(fmt::format("Could not create font face (file={})", file));

    FontFaceDict key(file, point);
    if (m_fontFaceMap.contains(key))
        FT_Done_Face(m_fontFaceMap[key]); // Delete existing face
    m_fontFaceMap[key] = fontFace;

    return fontFace;
}

FT_Face FontManager::getOrCreateFontFace(std::string file, float point) {
    if (auto fontFace = this->getFontFace(file, point)) return fontFace;
    else return this->createFontFace(file, point);
}
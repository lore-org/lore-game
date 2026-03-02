#include <engine/FontManager.h>

#include <cstdint>
#include <optional>

#include FT_FREETYPE_H
#include <simdutf.h>

#include <rectpack2D/finders_interface.h>
#include <rectpack2D/empty_spaces.h>

#include <engine/Engine.h>
#include <engine/utils.h>

#define log_freetype_error() do { if (auto s = FT_Error_String(e)) LogDebug(s); else LogDebug("FT_Error_String returned NULL"); } while (0)
#define log_simdutf_error() do { \
    switch (result.error) { \
        case simdutf::HEADER_BITS: LogDebug("simdutf: HEADER_BITS"); break; \
        case simdutf::TOO_SHORT: LogDebug("simdutf: TOO_SHORT"); break; \
        case simdutf::TOO_LONG: LogDebug("simdutf: TOO_LONG"); break; \
        case simdutf::OVERLONG: LogDebug("simdutf: OVERLONG"); break; \
        case simdutf::TOO_LARGE: LogDebug("simdutf: TOO_LARGE"); break; \
        case simdutf::SURROGATE: LogDebug("simdutf: SURROGATE"); break; \
        case simdutf::INVALID_BASE64_CHARACTER: LogDebug("simdutf: INVALID_BASE64_CHARACTER"); break; \
        case simdutf::BASE64_INPUT_REMAINDER: LogDebug("simdutf: BASE64_INPUT_REMAINDER"); break; \
        case simdutf::BASE64_EXTRA_BITS: LogDebug("simdutf: BASE64_EXTRA_BITS"); break; \
        case simdutf::OUTPUT_BUFFER_TOO_SMALL: LogDebug("simdutf: OUTPUT_BUFFER_TOO_SMALL"); break; \
        default: case simdutf::OTHER: LogDebug("simdutf: OTHER"); break; \
    } \
} while (0)

std::shared_ptr<FontManager> FontManager::m_instance;

FontManager::~FontManager() {
    // Run destructor before freetype library is deinitialised
    m_fontFaceMap.clear();

    if (auto e = FT_Done_FreeType(this->getFTLibrary())) {
        LogError("Could not deinitialise FreeType");
        log_freetype_error();
    }
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

FontManager::FontFace* FontManager::getFontFace(std::string file) {
    if (m_fontFaceMap.contains(file))
        return &m_fontFaceMap[file];

    return nullptr;
}

std::string FontManager::getFontFile(FontManager::FontFace* fontFace) {
    for (auto& storedFontFace : m_fontFaceMap) {
        if (storedFontFace.second == *fontFace) return storedFontFace.first;
    }

    return { };
}

FontManager::FontFace* FontManager::createFontFace(std::string file) {
    FT_Face ftFontFace;
    if (auto e = FT_New_Face(this->getFTLibrary(), file.c_str(), 0, &ftFontFace)) {
        LogError(fmt::format("Could not create font face (file={})", file));
        log_freetype_error();
        return nullptr;
    }

    FontFace fontFace(ftFontFace);

    m_fontFaceMap[file] = fontFace;
    return &m_fontFaceMap[file];
}

FontManager::FontFace* FontManager::getOrCreateFontFace(std::string file) {
    if (auto fontFace = this->getFontFace(file)) return fontFace;
    else return this->createFontFace(file);
}

FontManager::FontFace::FontFace(FT_Face font, float point) {
    m_ftFontFace = font;
    m_point = point;

    auto& pixelMode = font->glyph->bitmap.pixel_mode;
    switch (pixelMode) {
        case FT_PIXEL_MODE_GRAY:
            m_glyphAtlas = new Atlas(1024, 1);
            break;
        case FT_PIXEL_MODE_LCD:
        case FT_PIXEL_MODE_LCD_V:
            m_glyphAtlas = new Atlas(1024, 3);
            break;
        case FT_PIXEL_MODE_BGRA:
            m_glyphAtlas = new Atlas(1024, 4);
            break;
        default:
            LogError(fmt::format("Unsupported glyph render mode (mode={})", pixelMode));
            break;
    }
}

FontManager::FontFace::~FontFace() {
    delete m_glyphAtlas;

    if (auto e = FT_Done_Face(m_ftFontFace)) {
        LogError("Could not delete font face");
        log_freetype_error();
    }
}

bool FontManager::FontFace::operator==(FontFace second) {
    return m_ftFontFace == second.m_ftFontFace;
}

void FontManager::FontFace::setFontPoint(float point) {
    if (m_point == point) return;

    auto fontManager = FontManager::sharedManager();
    auto engine = Engine::sharedInstance();

    auto dpi = engine->getMonitorDPI(engine->getCurrentMonitor());
    if (auto e = FT_Set_Char_Size(this->m_ftFontFace, 0, point * 64, dpi.width, dpi.height)) {
        LogError(fmt::format("Could not resize font face (point={})", point));
        log_freetype_error();
    }

    m_renderedGlyphs.clear();
}

FontManager::Glyph* FontManager::FontFace::loadGlyph(char codepoint) {
    char32_t convertedCodepoint;
    auto result = simdutf::convert_utf8_to_utf32_with_errors(&codepoint, 1, &convertedCodepoint);
    if (result.is_err()) {
        LogError(fmt::format("Could not load glyph (codepoint={})", static_cast<int>(codepoint)));
        log_simdutf_error();
        return nullptr;
    }

    return FontFace::loadGlyph(convertedCodepoint);
}

FontManager::Glyph* FontManager::FontFace::loadGlyph(char16_t codepoint) {
    char32_t convertedCodepoint;
    auto result = simdutf::convert_utf16_to_utf32_with_errors(&codepoint, 1, &convertedCodepoint);
    if (result.is_err()) {
        LogError(fmt::format("Could not load glyph (codepoint={})", static_cast<int>(codepoint)));
        log_simdutf_error();
        return nullptr;
    }

    return FontFace::loadGlyph(convertedCodepoint);
}

FontManager::Glyph* FontManager::FontFace::loadGlyph(char32_t codepoint) {
    auto glyphIndex = FT_Get_Char_Index(m_ftFontFace, codepoint);
    if (auto e = FT_Load_Glyph(m_ftFontFace, glyphIndex, FT_LOAD_RENDER | FT_LOAD_COLOR)) {
        LogError(fmt::format("Could not create load glyph (codepoint={})", static_cast<uint32_t>(codepoint)));
        log_freetype_error();
        return nullptr;
    }

    // Look for cached glyphs
    for (auto& renderedGlyph : m_renderedGlyphs) {
        if (renderedGlyph.codepoint == codepoint) return &renderedGlyph;
    }


    auto& ftGlyph = m_ftFontFace->glyph;
    auto glyphRect = m_glyphAtlas->insertRect(ftGlyph->metrics.width, ftGlyph->metrics.height);

    m_glyphAtlas->drawPixels(glyphRect, reinterpret_cast<char*>(ftGlyph->bitmap.buffer));
    m_renderedGlyphs.push_back({
        m_ftFontFace,
        m_glyphAtlas,

        codepoint,
        glyphIndex,

        glyphRect.x, glyphRect.y,
        ftGlyph->metrics.horiBearingX, ftGlyph->metrics.horiBearingY,
        glyphRect.w, glyphRect.h,
        ftGlyph->metrics.horiAdvance
    });

    return &m_renderedGlyphs.back();
}

FontManager::Bitmap::Bitmap(int size, short channels)  {
    m_bitmap = static_cast<char*>(calloc(size * size, channels));
    m_bitmapSize = size;
    m_bitmapChannels = channels;
}

FontManager::Bitmap::~Bitmap() {
    free(m_bitmap);
}

void FontManager::Bitmap::resize(int size) {
    char* newBitmap = static_cast<char*>(calloc(size * size, m_bitmapChannels));
    const auto oldLineBytes = m_bitmapSize * m_bitmapChannels;
    const auto newLineBytes = size * m_bitmapChannels;

    if (size > m_bitmapSize) {
        // Expand Bitmap
        
        for (int y = 0; y < size; y++) {
            memcpy(
                newBitmap + (y * newLineBytes),
                m_bitmap + (y * oldLineBytes),
                oldLineBytes
            );
        }
    } else {
        // Shrink Bitmap

        for (int y = 0; y < size; y++) {
            memcpy(
                newBitmap + (y * newLineBytes),
                m_bitmap + (y * oldLineBytes),
                newLineBytes
            );
        }
    }

    free(m_bitmap);
    m_bitmap = newBitmap;
    m_bitmapSize = size;
}

char* FontManager::Bitmap::getPixel(int x, int y) {
    if (x > m_bitmapSize || y > m_bitmapSize) {
        LogError(fmt::format("Bitmap coordinates outside of range (x={}, y={})", x, y));
        return nullptr;
    }

    const auto xSkip = x * m_bitmapChannels;
    const auto ySkip = y * m_bitmapChannels * m_bitmapSize;

    return m_bitmap + xSkip + ySkip;
}

void FontManager::Bitmap::drawPixels(rect_t dimensions, char* data) {
    auto [ x, y, w, h ] = dimensions;
    for (int line = 0; line < h; line++) {
        memcpy(
            this->getPixel(x, y + line),
            data + ((y + line) * m_bitmapChannels * w),
            w * m_bitmapChannels
        );
    }
}

FontManager::Atlas::Atlas(int size, short channels) : m_packer({ size, size }) {
    m_packer.flipping_mode = rectpack2D::flipping_option::DISABLED;
}

rect_t FontManager::Atlas::insertRect(int width, int height) {
    // TODO - integrate this into the m_bitmap to draw an array to an inserted rect
    m_placedRects.push_back({ 0, 0, width, height });
    rect_t& rect = m_placedRects.back();

    do {
        std::optional<rect_t> insertedRect = m_packer.insert(rect.get_wh());
        if (insertedRect.has_value()) {
            rect = *insertedRect;
            break;
        } else {
            this->resize(m_bitmapSize + std::max(rect.w, rect.h));
        }
    } while (true);

    return rect;
}

void FontManager::Atlas::resize(int size) {
    Bitmap::resize(size);

    m_packer.reset({ size, size });
    for (auto rect : m_placedRects) {
        std::optional<rect_t> insertedRect = m_packer.insert(rect.get_wh());
        if (!insertedRect) {
            this->resize(size + std::max(rect.w, rect.h));
            break;
        }
    }
}
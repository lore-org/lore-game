#include "freetype/freetype.h"
#include <cstdint>
#include <engine/FontManager.h>

#include <iterator>
#include <simdutf.h>

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
    for (auto& storedFontFace : m_fontFaceMap) {
        if (auto e = FT_Done_Face(storedFontFace.second.ftFontFace)) {
            LogError("Could not delete font face");
            log_freetype_error();
        }
    }

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
    FontFace fontFace;

    if (auto e = FT_New_Face(this->getFTLibrary(), file.c_str(), 0, &fontFace.ftFontFace)) {
        LogError(fmt::format("Could not create font face (file={})", file));
        log_freetype_error();
        return nullptr;
    }
    fontFace.setFontPoint(12);

    if (m_fontFaceMap.contains(file)) {
        FT_Done_Face(m_fontFaceMap[file].ftFontFace); // Delete existing face
        if (m_renderedGlyphs.contains(fontFace.ftFontFace)) {
            // Clear cached glyphs
            m_renderedGlyphs[fontFace.ftFontFace].clear();
        }
    }
    m_fontFaceMap[file] = fontFace;

    return &m_fontFaceMap[file];
}

FontManager::FontFace* FontManager::getOrCreateFontFace(std::string file) {
    if (auto fontFace = this->getFontFace(file)) return fontFace;
    else return this->createFontFace(file);
}

void FontManager::FontFace::setFontPoint(float point) {
    if (m_point == point) return;

    auto fontManager = FontManager::sharedManager();
    auto engine = Engine::sharedInstance();

    auto dpi = engine->getMonitorDPI(engine->getCurrentMonitor());
    if (auto e = FT_Set_Char_Size(this->ftFontFace, 0, point * 64, dpi.width, dpi.height)) {
        LogError(fmt::format("Could not resize font face (point={})", point));
        log_freetype_error();
    }

    if (fontManager->m_renderedGlyphs.contains(this->ftFontFace)) {
        // Clear cached glyphs
        fontManager->m_renderedGlyphs[this->ftFontFace].clear();
    }
}

void FontManager::FontFace::loadGlyph(char codepoint) {
    char32_t convertedCodepoint;
    auto result = simdutf::convert_utf8_to_utf32_with_errors(&codepoint, 1, &convertedCodepoint);
    if (result.is_err()) {
        LogError(fmt::format("Could not load glyph (codepoint={})", static_cast<int>(codepoint)));
        log_simdutf_error();
        return;
    }

    FontFace::loadGlyph(convertedCodepoint);
}

void FontManager::FontFace::loadGlyph(char16_t codepoint) {
    char32_t convertedCodepoint;
    auto result = simdutf::convert_utf16_to_utf32_with_errors(&codepoint, 1, &convertedCodepoint);
    if (result.is_err()) {
        LogError(fmt::format("Could not load glyph (codepoint={})", static_cast<int>(codepoint)));
        log_simdutf_error();
        return;
    }

    FontFace::loadGlyph(convertedCodepoint);
}

void FontManager::FontFace::loadGlyph(char32_t codepoint) {
    auto glyphIndex = FT_Get_Char_Index(ftFontFace, codepoint);
    if (auto e = FT_Load_Glyph(ftFontFace, glyphIndex, FT_LOAD_RENDER | FT_LOAD_COLOR)) {
        LogError(fmt::format("Could not create load glyph (codepoint={})", static_cast<uint32_t>(codepoint)));
        log_freetype_error();
        return;
    }

    auto& ftGlyph = ftFontFace->glyph;

    Glyph glyph {
        ftFontFace,

        codepoint,
        glyphIndex,

        0, 0,
        ftGlyph->metrics.horiBearingX, ftGlyph->metrics.horiBearingY,
        ftGlyph->metrics.width, ftGlyph->metrics.height,
        ftGlyph->metrics.horiAdvance
    };

    FontManager::sharedManager()->m_renderedGlyphs[ftFontFace].push_back(glyph);
}

FontManager::Bitmap* FontManager::Bitmap::create(int size, short channels)  {
    auto bitmap = new Bitmap();
    bitmap->bitmapSize = size;
    bitmap->bitmapChannels = channels;
    bitmap->bitmap = static_cast<char*>(calloc(size * size, channels));

    return bitmap;
}

void FontManager::Bitmap::resize(int size) {
    char* newBitmap = static_cast<char*>(calloc(size * size, bitmapChannels));
    const auto oldLineBytes = this->bitmapSize * bitmapChannels;
    const auto newLineBytes = size * bitmapChannels;

    if (size > this->bitmapSize) {
        // Expand Bitmap
        
        for (int y = 0; y < size; y++) {
            memcpy(
                newBitmap + (y * newLineBytes),
                this->bitmap + (y * oldLineBytes),
                oldLineBytes
            );
        }
    } else {
        // Shrink Bitmap

        for (int y = 0; y < size; y++) {
            memcpy(
                newBitmap + (y * newLineBytes),
                this->bitmap + (y * oldLineBytes),
                newLineBytes
            );
        }
    }

    free(this->bitmap);
    this->bitmap = newBitmap;
    this->bitmapSize = size;
}

char* FontManager::Bitmap::getPixel(int x, int y) {
    if (x > bitmapSize || y > bitmapSize) {
        LogError(fmt::format("Bitmap coordinates outside of range (x={}, y={})", x, y));
        return nullptr;
    }

    const auto xSkip = x * bitmapChannels;
    const auto ySkip = y * bitmapChannels * bitmapSize;

    return this->bitmap + xSkip + ySkip;
}

FontManager::Atlas* FontManager::Atlas::create(int size, short channels) {
    auto atlas = new Atlas();
    atlas->bitmapSize = size;
    atlas->bitmapChannels = channels;
    atlas->bitmap = static_cast<char*>(calloc(size * size, channels));
    atlas->m_freeRectangles.push_back({ 0, 0, size, size });

    return atlas;
}

FontManager::Rect FontManager::Atlas::insertRect(int width, int height) {
    // TODO - integrate this into the bitmap to draw an array to an inserted rect
    Rect rect { 0, 0, width, height };

    for (int i = m_freeRectangles.size(); i >= 0; i--) {
        auto& freeRectangle = m_freeRectangles[i];
        if (rect.width > freeRectangle.width && rect.height > freeRectangle.height) {
            // TODO - evaluate all viable free rects and choose the one which leaves the least amount of area remaining
            rect.x = freeRectangle.x;
            rect.y = freeRectangle.y;

            std::vector<Rect> availableRectangles;
            const int verticalJoinArea = (freeRectangle.width - rect.width) * (freeRectangle.height);
            const int horizontalJoinArea = (freeRectangle.width) * (freeRectangle.height - rect.height);
            if (verticalJoinArea > horizontalJoinArea) {
                availableRectangles = {
                    {
                        rect.x + rect.width,
                        rect.y,
                        freeRectangle.width - rect.width,
                        freeRectangle.height
                    },
                    {
                        rect.x,
                        rect.y + rect.height,
                        rect.width,
                        freeRectangle.height - rect.height
                    }
                };
            } else {
                availableRectangles = {
                    {
                        rect.x + rect.width,
                        rect.y,
                        freeRectangle.width - rect.width,
                        rect.height
                    },
                    {
                        rect.x,
                        rect.y + rect.height,
                        freeRectangle.width,
                        freeRectangle.height - rect.height
                    }
                };
            }

            m_freeRectangles.erase(m_freeRectangles.begin() + i);
            std::move(
                availableRectangles.begin(), availableRectangles.end(),
                std::back_inserter(m_freeRectangles)
            );

            return rect;
        }
    }

    std::vector<Rect> availableRectangles {
        {
            bitmapSize,
            0,
            bitmapSize,
            bitmapSize * 2
        },
        {
            0,
            bitmapSize,
            bitmapSize,
            bitmapSize
        }
    };
    std::move(
        availableRectangles.begin(), availableRectangles.end(),
        std::back_inserter(m_freeRectangles)
    );

    this->resize(bitmapSize * 2);
    rect = this->insertRect(width, height);

    return rect;
}
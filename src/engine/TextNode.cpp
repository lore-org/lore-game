#include <engine/TextNode.h>

#include <cstring>

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <simdutf.h>

#include <engine/utils.h>
#include <engine/Engine.h>
#include <engine/FontManager.h>
#include <engine/Typeable.h>

TextNode::TextNode() :
    m_displayedText(""), m_fontPoint(DEFAULT_FONT_POINT),
    m_glProgram(glCreateProgram()),
    m_glVertexArray(0), m_glVertexBuffer(0), m_glTexture(0),
    m_numChars(0) {}

TextNode::~TextNode() {
    Engine::sharedInstance()->removeFramebufferUpdates(m_glProgram);

    glDeleteTextures(1, &m_glTexture);
    glDeleteBuffers(1, &m_glVertexBuffer);
    glDeleteVertexArrays(1, &m_glVertexArray);
    glDeleteProgram(m_glProgram);
}

bool TextNode::init(FT_Face font, float fontPoint, Point position) {
    if (!ColorNode::init()) return false;
    
    auto engine = Engine::sharedInstance();


    auto vertShader = engine->loadShaderFromFile(
        Engine::ShaderType::Vertex,
        "resources/shaders/text.vert"
    );

    auto fragShader = engine->loadShaderFromFile(
        Engine::ShaderType::Fragment,
        "resources/shaders/text.frag"
    );

    glBindFragDataLocation(m_glProgram, 0, "fragColor");
    glBindAttribLocation(m_glProgram, 1, "fragTexCoord");

    glAttachShader(m_glProgram, vertShader.shaderID);
    glAttachShader(m_glProgram, fragShader.shaderID);

    if (!utils::linkAndUseProgram(m_glProgram).success) return false;


    glGenBuffers(1, &m_glVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);


    glGenVertexArrays(1, &m_glVertexArray);
    glBindVertexArray(m_glVertexArray);

    auto vertPos = glGetAttribLocation(m_glProgram, "vertPos");
    glEnableVertexAttribArray(vertPos);
    glVertexAttribPointer(
        vertPos,
        2, GL_INT,
        GL_FALSE,
        sizeof(BufferData), reinterpret_cast<void*>(0)
    );

    auto texCoord = glGetAttribLocation(m_glProgram, "texCoord");
    glEnableVertexAttribArray(texCoord);
    glVertexAttribPointer(
        texCoord,
        2, GL_UNSIGNED_INT,
        GL_FALSE,
        sizeof(BufferData), reinterpret_cast<void*>(sizeof(BufferData::vertPos))
    );

    glUniform1i(glGetUniformLocation(m_glProgram, "glyphAtlas"), 0);

    engine->requestFramebufferUpdates(m_glProgram);

    m_fontPoint = fontPoint;
    this->setPosition(position);
    this->changeFont(font);


    return true;
}

// Font will default to `resources/Noto Sans.ttf` if not given
std::shared_ptr<TextNode> TextNode::create(FT_Face font, float fontPoint, Point position) {
    auto ret = utils::protected_make_shared<TextNode>();

    if (
        !ret->init(font, fontPoint, position)
    ) return nullptr;
    return ret;
}

std::shared_ptr<TextNode> TextNode::create(std::string fontFile, float fontPoint, Point position) {
    return TextNode::create(
        FontManager::sharedManager()->getOrCreateFontFace(
            fontFile, fontPoint
        ), fontPoint, position
    );
}

void TextNode::draw(const long double dt) {
    ColorNode::draw(dt);

    if (
        !this->isVisible() ||
        IsZero(this->getOpacity())
    ) return;


    glUseProgram(m_glProgram);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_glTexture);

    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glBindVertexArray(m_glVertexArray);


    if (m_statusBitset & UPDATE_VERTICES) {
        m_statusBitset &= ~UPDATE_VERTICES;

        this->_updateVertices();
        if (m_numChars <= 0) return;
    }

    if (m_statusBitset & UPDATE_COLOR) {
        m_statusBitset &= ~UPDATE_COLOR;
        
        glUniform4f(
            glGetUniformLocation(m_glProgram, "textColor"),
            m_color.r / 255., m_color.g / 255., m_color.b / 255., m_color.a / 255.
        );
    }

    if (m_statusBitset & UPDATE_ROTATION) {
        m_statusBitset &= ~UPDATE_ROTATION;
        
        glUniform1f(
            glGetUniformLocation(m_glProgram, "rotateDeg"),
            m_rotation
        );
    }

    if (m_statusBitset & UPDATE_ATLAS) {
        m_statusBitset &= ~UPDATE_ATLAS;

        this->_createAtlasTex();
    }


    glDrawArrays(GL_TRIANGLES, 0, m_numChars * 6);
}

void TextNode::setDisplayedText(std::string displayedText) {
    m_displayedText = displayedText;
    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::setFontPoint(float fontPoint) {
    m_fontPoint = fontPoint;

    auto font = FontManager::sharedManager()->getFontDict(m_fontFace);
    this->changeFont(font.first);
}

void TextNode::changeFont(std::string fontFile) {
    this->changeFont(FontManager::sharedManager()->getOrCreateFontFace(
        fontFile, m_fontPoint
    ));
}

// Making font `nullptr` will default to `resources/Noto Sans.ttf`
void TextNode::changeFont(FT_Face font) {
    if (!font) {
        font = FontManager::sharedManager()->getOrCreateFontFace(
            "resources/Noto Sans.ttf", m_fontPoint
        );
    } else {
        auto fontDict = FontManager::sharedManager()->getFontDict(m_fontFace);
        font = FontManager::sharedManager()->getOrCreateFontFace(
            fontDict.first, m_fontPoint
        );
    }

    m_fontFace = font;

    m_statusBitset |= UPDATE_VERTICES | UPDATE_ATLAS;
}

void TextNode::_createAtlasTex() {
    // glUseProgram(m_glProgram);

    // if (!m_glTexture) glGenTextures(1, &m_glTexture);

    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, m_glTexture);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // auto bitmap = m_fontAtlas->GetBitmap();
    
    // glUniform2f(
    //     glGetUniformLocation(m_glProgram, "atlasSize"),
    //     bitmap.Width(), bitmap.Height()
    // );


    // GLint internalFormat;
    // GLint format;

    // switch (bitmap.Channels()) {
    //     default:
    //     case 1:
    //         internalFormat = GL_R8;
    //         format = GL_RED;
    //         break;
    //     case 2:
    //         internalFormat = GL_RG8;
    //         format = GL_RG;
    //         break;
    //     case 3:
    //         internalFormat = GL_RGB8;
    //         format = GL_RGB;
    //         break;
    //     case 4:
    //         internalFormat = GL_RGBA8;
    //         format = GL_RGBA;
    //         break;
    // }

    // glTexImage2D(
    //     GL_TEXTURE_2D, 0,
    //     format,
    //     bitmap.Width(), bitmap.Height(),
    //     0,
    //     format, GL_UNSIGNED_BYTE,
    //     bitmap.Data().data()
    // );
}

void TextNode::_updateVertices() {
    // auto shapedGlyphs = m_fontTextShaper->ShapeUtf8(m_displayedText);
    // m_numChars = shapedGlyphs.size();


    // glUseProgram(m_glProgram);

    // glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    // glBindVertexArray(m_glVertexArray);
    
    // glBufferData(
    //     GL_ARRAY_BUFFER,
    //     sizeof(BufferData[6]) * m_numChars, NULL,
    //     GL_STATIC_DRAW
    // );
    
    
    // auto shaperMeasurement = Trex::TextShaper::Measure(shapedGlyphs);
    // auto fontMetrics = m_fontTextShaper->GetFontMetrics();

    // this->setContentSize(shaperMeasurement.width, fontMetrics.ascender - fontMetrics.descender);

    // if (m_numChars <= 0) return;

    // auto framebufferHeight = FontManager::sharedManager()->getFrameBufferHeight();
    // auto rect = this->getRect();
    // auto fontMeasurement = Trex::TextShaper::Measure(m_fontTextShaper->ShapeAscii("\xDB")); // "█"

    // auto xpos = rect.getMinX();
    // auto ypos = framebufferHeight - rect.getMinY() - fontMeasurement.height;

    // glUniform2f(
    //     glGetUniformLocation(m_glProgram, "rectOrigin"),
    //     xpos + (rect.getWidth() * this->getAnchorX()),
    //     ypos - (rect.getHeight() * this->getAnchorY())
    // );

    // for (size_t i = 0; i < m_numChars; i++) {
    //     auto& shapedGlyph = shapedGlyphs[i];
    //     auto& glyph = shapedGlyph.info;


    //     int x = xpos + shapedGlyph.xOffset + glyph.bearingX;
    //     int y = ypos - shapedGlyph.yOffset + glyph.bearingY;
    //     int w = glyph.width;
    //     int h = glyph.height;

    //     unsigned int texX = glyph.x;
    //     unsigned int texY = glyph.y;
    //     unsigned int texW = glyph.width;
    //     unsigned int texH = glyph.height;
        

    //     // Will be flipped across x-axis in vertex shader
    //     /*
    //         0     4---3
    //         |\     \  |
    //         | \     \ |
    //         |  \     \|
    //         1---2     5
    //     */
    //     BufferData data[6] {
    //         { { x    , y     }, { texX       , texY        } },
    //         { { x    , y - h }, { texX       , texY + texH } },
    //         { { x + w, y - h }, { texX + texW, texY + texH } },

    //         { { x + w, y     }, { texX + texW, texY        } },
    //         { { x    , y     }, { texX       , texY        } },
    //         { { x + w, y - h }, { texX + texW, texY + texH } }
    //     };

    //     glBufferSubData(
    //         GL_ARRAY_BUFFER,
    //         sizeof(data) * i, sizeof(data), 
    //         data
    //     );


    //     xpos += shapedGlyph.xAdvance;
    //     ypos -= shapedGlyph.yAdvance;
    // }

    
    // if (auto typeable = dynamic_pointer_cast<Typeable>(m_parent)) {
    //     typeable->_measureString();
    // }
}

void TextNode::setScale(long double scale) {
    ColorNode::setScale(scale);
    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::setRotation(long double rotation) {
    ColorNode::setRotation(rotation);
    m_statusBitset |= UPDATE_ROTATION;
}

void TextNode::setPosition(long double x, long double y) {
    ColorNode::setPosition(x, y);
    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::setAnchorPoint(long double x, long double y) {
    ColorNode::setAnchorPoint(x, y);
    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::setContentSize(long double width, long double height) {
    ColorNode::setContentSize(width, height);
    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::setColorA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    ColorNode::setColorA(r, g, b, a);
    m_statusBitset |= UPDATE_COLOR;
}
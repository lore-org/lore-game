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
    m_displayedText(""),
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

bool TextNode::init(FontManager::FontFace* font, Point position) {
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

    this->setPosition(position);
    this->changeFont(font);


    return true;
}

// Font will default to `resources/Noto Sans.ttf` if not given
std::shared_ptr<TextNode> TextNode::create(FontManager::FontFace* font, Point position) {
    auto ret = utils::protected_make_shared<TextNode>();

    if (
        !ret->init(font, position)
    ) return nullptr;
    return ret;
}

std::shared_ptr<TextNode> TextNode::create(std::string fontFile, Point position) {
    return TextNode::create(
        FontManager::sharedManager()->getOrCreateFontFace(fontFile),
        position
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

        this->_createAtlasTex();
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


    glDrawArrays(GL_TRIANGLES, 0, m_numChars * 6);
}

void TextNode::setDisplayedText(std::string displayedText) {
    m_displayedText = displayedText;
    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::setFontPoint(float point) {
    m_fontFace->setFontPoint(point);
    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::changeFont(std::string fontFile) {
    this->changeFont(FontManager::sharedManager()->getOrCreateFontFace(fontFile));
}

// Making font `nullptr` will default to `resources/Noto Sans.ttf`
void TextNode::changeFont(FontManager::FontFace* font) {
    if (!font) {
        font = FontManager::sharedManager()->getOrCreateFontFace("resources/Noto Sans.ttf");
    }

    m_fontFace = font;

    m_statusBitset |= UPDATE_VERTICES;
}

void TextNode::_createAtlasTex() {
    glUseProgram(m_glProgram);

    if (!m_glTexture) glGenTextures(1, &m_glTexture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_glTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto& atlas = m_fontFace->m_glyphAtlas;
    
    glUniform2f(
        glGetUniformLocation(m_glProgram, "atlasSize"),
        atlas->m_bitmapSize, atlas->m_bitmapSize
    );


    GLint internalFormat;
    GLint format;

    switch (atlas->m_bitmapChannels) {
        default:
        case 1:
            internalFormat = GL_R8;
            format = GL_RED;
            break;
        case 2: // Should be unused
            internalFormat = GL_RG8;
            format = GL_RG;
            break;
        case 3:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            break;
        case 4:
            internalFormat = GL_RGBA8;
            format = GL_BGRA;
            break;
    }

    glTexImage2D(
        GL_TEXTURE_2D, 0,
        internalFormat,
        atlas->m_bitmapSize, atlas->m_bitmapSize,
        0,
        format, GL_UNSIGNED_BYTE,
        atlas->m_bitmap
    );
}

void TextNode::_updateVertices() {
    auto glyphs = m_fontFace->loadString(m_displayedText);
    m_numChars = glyphs.size();


    glUseProgram(m_glProgram);

    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glBindVertexArray(m_glVertexArray);
    
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(BufferData[6]) * m_numChars, NULL,
        GL_STATIC_DRAW
    );
    
    int width = 0;
    int height = m_fontFace->m_globalAscender;
    for (size_t i = 0; i < m_numChars; i++) {
        auto& glyph = glyphs[i];
        if (i < m_numChars - 1) width += glyph->advanceX;
        else width += glyph->offsetX + glyph->advanceX;
    }

    this->setContentSize(width, height);

    if (m_numChars <= 0) return;

    auto framebufferHeight = Engine::sharedInstance()->getFrameBufferHeight();
    auto rect = this->getRect();

    auto xpos = rect.getMinX();
    auto ypos = framebufferHeight - rect.getMinY() - height;

    glUniform2f(
        glGetUniformLocation(m_glProgram, "rectOrigin"),
        xpos + (rect.getWidth() * this->getAnchorX()),
        ypos - (rect.getHeight() * this->getAnchorY())
    );

    for (size_t i = 0; i < m_numChars; i++) {
        auto& glyph = glyphs[i];

        int x = xpos + glyph->offsetX;
        int y = ypos - glyph->offsetY;
        int w = glyph->width;
        int h = glyph->height;

        unsigned int texX = glyph->atlasX;
        unsigned int texY = glyph->atlasY;
        unsigned int texW = glyph->atlasWidth;
        unsigned int texH = glyph->atlasHeight;
        

        // Will be flipped across x-axis in vertex shader
        /*
            0     4---3
            |\     \  |
            | \     \ |
            |  \     \|
            1---2     5
        */
        BufferData data[6] {
            { { x    , y     }, { texX       , texY        } },
            { { x    , y - h }, { texX       , texY + texH } },
            { { x + w, y - h }, { texX + texW, texY + texH } },

            { { x + w, y     }, { texX + texW, texY        } },
            { { x    , y     }, { texX       , texY        } },
            { { x + w, y - h }, { texX + texW, texY + texH } }
        };

        glBufferSubData(
            GL_ARRAY_BUFFER,
            sizeof(data) * i, sizeof(data), 
            data
        );


        xpos += glyph->advanceX;
    }

    
    if (auto typeable = dynamic_pointer_cast<Typeable>(m_parent)) {
        typeable->_measureString();
    }
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
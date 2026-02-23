#include <engine/Sprite.h>

#include <memory>
#include <regex>

#if __ANDROID__
    #include <glad/gles1.h>
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <discord-rpc.hpp>

#include <ada.h>

#include <httplib.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <engine/config.hpp>
#include <engine/utils.h>
#include <engine/Geometry.h>
#include <engine/ColorNode.h>
#include <engine/Engine.h>
#include <engine/Geometry.h>

Sprite::Sprite() :
    m_texture(nullptr),
    m_glProgram(glCreateProgram()),
    m_glVertexArray(0), m_glVertexBuffer(0), m_glTexture(0) {}

Sprite::~Sprite() {    
    stbi_image_free(m_texture->data);
    delete m_texture;

    Engine::sharedInstance()->removeFramebufferUpdates(m_glProgram);

    glDeleteTextures(1, &m_glTexture);
    glDeleteBuffers(1, &m_glVertexBuffer);
    glDeleteVertexArrays(1, &m_glVertexArray);
    glDeleteProgram(m_glProgram);
}

bool Sprite::init(Texture* texture) {
    if (!ColorNode::init()) return false;
    if (!texture) return false;
    
    auto engine = Engine::sharedInstance();


    auto vertShader = engine->loadShaderFromFile(
        Engine::ShaderType::Vertex,
        "resources/shaders/sprite.vert"
    );

    auto fragShader = engine->loadShaderFromFile(
        Engine::ShaderType::Fragment,
        "resources/shaders/sprite.frag"
    );

    glBindFragDataLocation(m_glProgram, 0, "fragColor");
    glBindAttribLocation(m_glProgram, 1, "fragTexCoord");

    glAttachShader(m_glProgram, vertShader.shaderID);
    glAttachShader(m_glProgram, fragShader.shaderID);

    if (!utils::linkAndUseProgram(m_glProgram).success) return false;


    glGenBuffers(1, &m_glVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(BufferData[6]), NULL, GL_STATIC_DRAW);


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
    
    glUniform1i(glGetUniformLocation(m_glProgram, "spriteTex"), 0);

    engine->requestFramebufferUpdates(m_glProgram);

    this->setTexture(texture);
    this->setContentSize(m_texture->width, m_texture->height);

    return true;
}

std::shared_ptr<Sprite> Sprite::create() {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init({})) return nullptr;
    return ret;
}

std::shared_ptr<Sprite> Sprite::createFromFile(std::string filename) {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init(Sprite::loadFromFile(filename))) return nullptr;
    return ret;
}

std::shared_ptr<Sprite> Sprite::createFromURL(std::string url) {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init(Sprite::loadFromURL(url))) return nullptr;
    return ret;
}

std::shared_ptr<Sprite> Sprite::createFromSprite(std::shared_ptr<Sprite> sprite) {
    auto ret = utils::protected_make_shared<Sprite>();

    if (!ret->init(sprite->getTexture())) return nullptr;
    return ret;
}

void Sprite::setTexture(Texture* texture) {
    if (m_texture) delete m_texture;
    m_texture = texture;

    m_statusBitset |= UPDATE_TEXTURE;
}

void Sprite::draw(const long double dt) {
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
    }

    if (m_statusBitset & UPDATE_COLOR) {
        m_statusBitset &= ~UPDATE_COLOR;
        
        glUniform4f(
            glGetUniformLocation(m_glProgram, "color"),
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

    if (m_statusBitset & UPDATE_TEXTURE) {
        m_statusBitset &= ~UPDATE_TEXTURE;

        this->_createTexture();
    }


    glDrawArrays(GL_TRIANGLES, 0, 6);
}

Sprite::Texture* Sprite::loadFromURL(std::string url) {
    auto parsedUrl = ada::parse(url);
    httplib::Client client(fmt::format("{}//{}", parsedUrl->get_protocol(), parsedUrl->get_host()));

    auto response = client.Get(static_cast<std::string>(parsedUrl->get_pathname()));
    if (response->status != 200) {
        LogError(fmt::format("Image returned status code '{}'", response->status));
        return nullptr;
    }

    auto tex = new Texture();
    tex->data = stbi_load_from_memory(
        reinterpret_cast<unsigned char*>(response->body.data()), response->body.size(),
        &tex->width, &tex->height,
        reinterpret_cast<int*>(&tex->channels), 0
    );

    return tex;
}

Sprite::Texture* Sprite::loadFromFile(std::string filename) {
    auto tex = new Texture();
    tex->data = stbi_load(
        filename.c_str(),
        &tex->width, &tex->height,
        reinterpret_cast<int*>(&tex->channels), 0
    );

    return tex;
}

void Sprite::_createTexture() {
    glUseProgram(m_glProgram);

    if (!m_glTexture) glGenTextures(1, &m_glTexture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_glTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUniform1i(
        glGetUniformLocation(m_glProgram, "colorChannels"),
        m_texture->channels
    );


    GLint internalFormat;
    GLint format;

    switch (m_texture->channels) {
        default:
        case Texture::Channels::Grayscale:
            internalFormat = GL_R8;
            format = GL_RED;
            break;
        case Texture::Channels::GrayscaleAlpha:
            internalFormat = GL_RG8;
            format = GL_RG;
            break;
        case Texture::Channels::RGB:
            internalFormat = GL_RGB8;
            format = GL_RGB;
            break;
        case Texture::Channels::RGBA:
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
            break;
    }

    glTexImage2D(
        GL_TEXTURE_2D, 0,
        format,
        m_texture->width, m_texture->height,
        0,
        format, GL_UNSIGNED_BYTE,
        m_texture->data
    );
}

void Sprite::_updateVertices() {
    glUseProgram(m_glProgram);

    glBindVertexArray(m_glVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);


    auto rect = this->getRect();
    auto framebufferHeight = Engine::sharedInstance()->getFrameBufferHeight();

    int x = rect.getMinX();
    int y = framebufferHeight - rect.getMinY();
    int w = rect.getWidth();
    int h = rect.getHeight();

    glUniform2f(
        glGetUniformLocation(m_glProgram, "rectOrigin"),
        x + (w * this->getAnchorX()),
        y - (h * this->getAnchorY())
    );

    // Will be flipped across x-axis in vertex shader
    /*
        0     4---3
        |\     \  |
        | \     \ |
        |  \     \|
        1---2     5
    */
    BufferData data[6] {
        { { x    , y     }, { 0, 0 } },
        { { x    , y - h }, { 0, 1 } },
        { { x + w, y - h }, { 1, 1 } },

        { { x + w, y     }, { 1, 0 } },
        { { x    , y     }, { 0, 0 } },
        { { x + w, y - h }, { 1, 1 } }
    };

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0, sizeof(data),
        data
    );
}

void Sprite::setScale(long double scale) {
    ColorNode::setScale(scale);
    m_statusBitset |= UPDATE_VERTICES;
}

void Sprite::setRotation(long double rotation) {
    ColorNode::setRotation(rotation);
    m_statusBitset |= UPDATE_ROTATION;
}

void Sprite::setPosition(long double x, long double y) {
    ColorNode::setPosition(x, y);
    m_statusBitset |= UPDATE_VERTICES;
}

void Sprite::setAnchorPoint(long double x, long double y) {
    ColorNode::setAnchorPoint(x, y);
    m_statusBitset |= UPDATE_VERTICES;
}

void Sprite::setContentSize(long double width, long double height) {
    ColorNode::setContentSize(width, height);
    m_statusBitset |= UPDATE_VERTICES;
}

void Sprite::setColorA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    ColorNode::setColorA(r, g, b, a);
    m_statusBitset |= UPDATE_COLOR;
}
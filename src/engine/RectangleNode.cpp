#include <furredengine/RectangleNode.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <furredengine/Engine.h>
#include <furredengine/Geometry.h>
#include <furredengine/utils.h>
#include <furredengine/Engine.h>

using namespace FurredEngine;

RectangleNode::RectangleNode() :
    m_filled(true),
    m_glProgram(glCreateProgram()),
    m_glVertexArray(0), m_glVertexBuffer(0) {}

RectangleNode::~RectangleNode() {
    Engine::sharedInstance()->removeFramebufferUpdates(m_glProgram);
    
    glDeleteBuffers(1, &m_glVertexBuffer);
    glDeleteVertexArrays(1, &m_glVertexArray);
    glDeleteProgram(m_glProgram);
}

bool RectangleNode::init(FurredEngine::Point origin, FurredEngine::Size size) {
    if (!ColorNode::init()) return false;

    auto engine = Engine::sharedInstance();


    auto vertShader = engine->loadShaderFromFile(
        Engine::ShaderType::Vertex,
        "resources/shaders/rect.vert"
    );

    auto fragShader = engine->loadShaderFromFile(
        Engine::ShaderType::Fragment,
        "resources/shaders/rect.frag"
    );

    glBindFragDataLocation(m_glProgram, 0, "fragColor");

    glAttachShader(m_glProgram, vertShader.shaderID);
    glAttachShader(m_glProgram, fragShader.shaderID);

    if (!utils::linkAndUseProgram(m_glProgram).success) return false;


    glGenBuffers(1, &m_glVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(BufferData[6]), NULL, GL_STATIC_DRAW);


    glGenVertexArrays(1, &m_glVertexArray);
    glBindVertexArray(m_glVertexArray);

    auto vertexPos = glGetAttribLocation(m_glProgram, "vertPos");
    glEnableVertexAttribArray(vertexPos);
    glVertexAttribPointer(
        vertexPos,
        2, GL_FLOAT,
        GL_FALSE,
        sizeof(BufferData), reinterpret_cast<void*>(0)
    );


    engine->requestFramebufferUpdates(m_glProgram);

    ColorNode::setPosition(origin);
    ColorNode::setContentSize(size);

    return true;
}

std::shared_ptr<RectangleNode> RectangleNode::create() {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(0, 0)) return nullptr;
    return ret;
}

std::shared_ptr<RectangleNode> RectangleNode::createWithVec(FurredEngine::Point origin, FurredEngine::Size size) {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(origin, size)) return nullptr;
    return ret;
}

std::shared_ptr<RectangleNode> RectangleNode::createWithRect(FurredEngine::Rect rectangle) {
    auto ret = utils::protected_make_shared<RectangleNode>();

    if (!ret->init(rectangle.origin, rectangle.size)) return nullptr;
    return ret;
}

void RectangleNode::draw(const long double dt) {
    ColorNode::draw(dt);

    if (
        !this->isVisible() ||
        IsZero(this->getOpacity())
    ) return;


    glUseProgram(m_glProgram);

    glBindVertexArray(m_glVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);


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


    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RectangleNode::_updateVertices() {
    glUseProgram(m_glProgram);

    glBindVertexArray(m_glVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);


    auto rect = this->getRect();
    auto framebufferHeight = Engine::sharedInstance()->getFrameBufferHeight();

    float x = rect.getMinX();
    float y = framebufferHeight - rect.getMinY();
    float w = rect.getWidth();
    float h = rect.getHeight();

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
        { { x    , y     } },
        { { x    , y - h } },
        { { x + w, y - h } },

        { { x + w, y     } },
        { { x    , y     } },
        { { x + w, y - h } }
    };

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0, sizeof(data),
        data
    );
}

void RectangleNode::setScale(long double scale) {
    ColorNode::setScale(scale);
    m_statusBitset |= UPDATE_VERTICES;
}

void RectangleNode::setRotation(long double rotation) {
    ColorNode::setRotation(rotation);
    m_statusBitset |= UPDATE_ROTATION;
}

void RectangleNode::setPosition(long double x, long double y) {
    ColorNode::setPosition(x, y);
    m_statusBitset |= UPDATE_VERTICES;
}

void RectangleNode::setAnchorPoint(long double x, long double y) {
    ColorNode::setAnchorPoint(x, y);
    m_statusBitset |= UPDATE_VERTICES;
}

void RectangleNode::setContentSize(long double width, long double height) {
    ColorNode::setContentSize(width, height);
    m_statusBitset |= UPDATE_VERTICES;
}

void RectangleNode::setColorA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    ColorNode::setColorA(r, g, b, a);
    m_statusBitset |= UPDATE_COLOR;
}
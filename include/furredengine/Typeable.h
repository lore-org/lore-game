#pragma once

#include <memory>
#include <cstdint>
#include <string>

#include <furredengine/Geometry.h>
#include <furredengine/ColorNode.h>
#include <furredengine/RectangleNode.h>
#include <furredengine/Touchable.h>
#include <furredengine/Engine.h>
#include <furredengine/utils.h>

namespace FurredEngine {

class TextNode;

class Typeable : public Touchable {
    friend class Engine;
    friend class TextNode;

public:
    struct Events {
        // Input of node was modified. Provides std::string
        CreateEventDecl(typeable, changed);
        // Text content of node was sought. Provides Typeable::SeekBounds
        CreateEventDecl(typeable, seeked);
        
        // Node gained focus
        CreateEventDecl(typeable, focusin);
        // Node lost focus
        CreateEventDecl(typeable, focusout);
    };

    enum class InputType {
        Text,               /**< The input is text */
        Name,               /**< The input is a person's name */
        Email,              /**< The input is an e-mail address */
        Username,           /**< The input is a username */
        HiddenPassword,     /**< The input is a secure password that is hidden */
        VisiblePassword,    /**< The input is a secure password that is visible */
        Number,             /**< The input is a number */
        HiddenPIN,          /**< The input is a secure PIN that is hidden */
        VisiblePIN          /**< The input is a secure PIN that is visible */
    };

    struct SeekBounds {
        int32_t start;
        int32_t length;
    };

    virtual bool init(ColorNode::Color4 displayTextColor, ColorNode::Color4 placeholderTextColor);

    static std::shared_ptr<Typeable> create();
    static std::shared_ptr<Typeable> createWithColors(ColorNode::Color4 displayTextColor, ColorNode::Color4 placeholderTextColor);

    void setInputType(InputType type);
    inline InputType getInputType() { return m_inputType; }

    virtual void update(const long double dt) override;
    virtual void draw(const long double dt) override;    

    std::shared_ptr<TextNode> m_displayText;
    std::shared_ptr<TextNode> m_placeholderText;
    std::shared_ptr<RectangleNode> m_cursor;


    // overrides for statusBitset

    virtual void setScale(long double scale) override;
    
    virtual void setPosition(long double x, long double y) override;
    virtual inline void setPosition(FurredEngine::Point position) override { setPosition(position.x, position.y); }

    virtual void setAnchorPoint(long double x, long double y) override;
    virtual inline void setAnchorPoint(FurredEngine::Point anchorPoint) override { setAnchorPoint(anchorPoint.x, anchorPoint.y); }

    virtual void setContentSize(long double width, long double height) override;
    virtual inline void setContentSize(FurredEngine::Size contentSize) override { setContentSize(contentSize.width, contentSize.height); }

protected:
    Typeable();

    InputType m_inputType;
    SeekBounds m_seekBounds;

    float m_widthToCursor;

private:
    enum : char {
        UPDATE_TEXT =           1 << 0,
        UPDATE_CURSOR =         1 << 1,
        UPDATE_BACKGROUND =     1 << 2
    };
    char m_statusBitset = 0b111;

    void _focusIn(void* data);
    void _focusOut(void* data);

    enum class DeleteType : uint8_t {
        Backwards, // Backspace
        Forwards // Delete
    };

    void _handleText(std::string text);
    void _handleDelete(DeleteType type);
    void _handleSeeking(int32_t start, int32_t length);

    void _measureString();

    void _updateChildren();
};

}
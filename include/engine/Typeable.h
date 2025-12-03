#pragma once

#include <memory>

#include <SDL3/SDL.h>

#include <engine/ColorNode.h>
#include <engine/RectangleNode.h>
#include <engine/Touchable.h>

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

    enum class InputType : SDL_PropertiesID {
        Text = SDL_TEXTINPUT_TYPE_TEXT,                                 /**< The input is text */
        Name = SDL_TEXTINPUT_TYPE_TEXT_NAME,                            /**< The input is a person's name */
        Email = SDL_TEXTINPUT_TYPE_TEXT_EMAIL,                          /**< The input is an e-mail address */
        Username = SDL_TEXTINPUT_TYPE_TEXT_USERNAME,                    /**< The input is a username */
        HiddenPassword = SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN,       /**< The input is a secure password that is hidden */
        VisiblePassword = SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_VISIBLE,     /**< The input is a secure password that is visible */
        Number = SDL_TEXTINPUT_TYPE_NUMBER,                             /**< The input is a number */
        HiddenPIN = SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_HIDDEN,           /**< The input is a secure PIN that is hidden */
        VisiblePIN = SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_VISIBLE         /**< The input is a secure PIN that is visible */
    };

    struct SeekBounds {
        int32_t start;
        int32_t length;
    };

    virtual bool init(ColorNode::Color4 displayTextColor, ColorNode::Color4 placeholderTextColor, ColorNode::Color4 backgroundColor);

    static std::shared_ptr<Typeable> create();
    static std::shared_ptr<Typeable> createWithColors(ColorNode::Color4 displayTextColor, ColorNode::Color4 placeholderTextColor, ColorNode::Color4 backgroundColor);

    void setInputType(InputType type);
    inline InputType getInputType() { return m_inputType; }

    virtual void update(const long double dt) override;
    virtual void draw(const long double dt) override;    

    std::shared_ptr<TextNode> m_displayText;
    std::shared_ptr<TextNode> m_placeholderText;
    std::shared_ptr<RectangleNode> m_cursor;
    std::shared_ptr<RectangleNode> m_background;

protected:
    Typeable();

    InputType m_inputType;
    SeekBounds m_seekBounds;

    int m_widthToCursor;

private:
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
};
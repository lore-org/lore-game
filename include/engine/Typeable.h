#pragma once

#include <engine/Touchable.h>

#include <SDL3/SDL.h>

class Typeable : public Touchable {
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
        HiddenPIN, SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_HIDDEN,           /**< The input is a secure PIN that is hidden */
        VisiblePIN = SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_VISIBLE         /**< The input is a secure PIN that is visible */
    };

    struct SeekBounds {
        int32_t start;
        int32_t length;
    };

    virtual bool init() override;

    static std::shared_ptr<Typeable> create();

    void setInputText(std::string text);
    inline std::string getInputText() { return m_inputText; }

    void setPlaceholderText(std::string text);
    inline std::string getPlaceholderText() { return m_placeholderText; }

    void setInputType(InputType type);
    inline InputType getInputType() { return m_inputType; }    

    virtual void update(const long double dt) override;
    virtual void draw(const long double dt) override;

protected:
    Typeable();

    std::string m_inputText;
    std::string m_placeholderText;

    InputType m_inputType;

    SeekBounds m_seekBounds = { 0, 0 };

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

    friend void Engine::runEngine();
};
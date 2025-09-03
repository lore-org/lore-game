#include "SDL3/SDL_keyboard.h"
#include <engine/Touchable.h>

class Typeable : public Touchable {
public:
    struct Events {
        // Input of node was modified
        CreateEventDecl(typeable, changed);
    };

    enum InputType {
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

    virtual bool init() override;

    void setInputText(std::string text);
    inline std::string getInputText() { return m_inputText; }

    void setPlaceholderText(std::string text);
    inline std::string getPlaceholderText() { return m_placeholderText; }

    void setInputType(InputType type);
    inline InputType getInputType() { return m_inputType; }

    virtual void update(const long double dt) override;

protected:
    Typeable();

    std::string m_inputText;
    std::string m_placeholderText;

    InputType m_inputType;

private:
    static void _focusIn(std::shared_ptr<void> data);
    static void _focusOut(std::shared_ptr<void> data);
};
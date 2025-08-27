#include <engine/Touchable.h>

class Typeable : public Touchable {
public:
    struct Events {
        // Input of node was modified
        CreateEventDecl(typeable, changed);
    };

    inline std::string getInputText() { return m_inputText; }
    inline std::string getPlaceholderText() { return m_placeholderText; }

protected:
    Typeable();

    std::string m_inputText;
    std::string m_placeholderText;
};
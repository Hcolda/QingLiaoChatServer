#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <memory>

class InputImpl;
namespace qls
{
    class Input
    {
    public:
        Input() = default;
        ~Input() = default;

        void init();

        bool input(const std::string& command);

    private:
        std::shared_ptr<InputImpl> m_impl;
    };
}

#endif // !INPUT_H
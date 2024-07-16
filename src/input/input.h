#ifndef INPUT_H
#define INPUT_H

#include <string>

namespace qls
{
    class Input
    {
    public:
        Input() = default;
        ~Input() = default;

        bool input(const std::string& command);
    };
}

#endif // !INPUT_H
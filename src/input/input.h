#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <memory>

class InputImpl;
namespace qls
{

class Input final
{
public:
    Input() = default;
    ~Input() = default;

    void init();

    bool input(const std::string& command);

private:
    std::shared_ptr<InputImpl> m_impl;
};

} // namespace qls

#endif // !INPUT_H
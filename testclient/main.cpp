#include <iostream>

#include "network.h"

int main() {
    qls::Network network;
    try
    {
        network.connect();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}

#include <iostream>
#include <system_error>
#include <chrono>
#include <atomic>

#include "network.h"
#include <option.hpp>

int main() {
    qls::Network network;
    std::atomic<bool> can_be_used = false;

    network.add_connected_error_callback("connected_error_callback", [](std::error_code ec){
        std::cerr << "Connected error: " << ec.message() << '\n';
        exit(-1);
    });
    network.add_connected_callback("connected_callback", [&](){
        std::cout << "Connected to server successfully!\n";
        can_be_used = true;
    });
    network.add_received_stdstring_callback("received_stdstring_callback", [](std::string message){
        std::cout << "Message received: " << message << '\n';
    });

    network.connect();
    std::cout << "Connecting to server...\n";
    while (true)
    {
        if (!can_be_used)
        {
            using namespace std::chrono;
            std::this_thread::sleep_for(0.1s);
            continue;
        }
        std::string command;
        std::cin >> command;
        if (command == "stop")
        {
            return 0;
        }
    }
}

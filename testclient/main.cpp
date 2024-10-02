#include <iostream>
#include <system_error>
#include <chrono>
#include <atomic>

#include "network.h"
#include "session.h"
#include <option.hpp>

static std::string strip(std::string_view data)
{
    std::string_view::const_iterator first = data.cbegin();
    auto last = data.crbegin();

    while (first != data.cend() && *first == ' ') ++first;
    while (last != data.crend() && *last == ' ') ++last;

    if (first >= last.base()) return {};
    return { first, last.base() };
}

static std::vector<std::string> split(std::string_view data)
{
    std::vector<std::string> dataList;

    long long begin = -1;
    long long i = 0;

    for (; static_cast<size_t>(i) < data.size(); i++) {
        if (data[i] == ' ') {
            if ((i - begin - 1) > 0)
                dataList.emplace_back(data.begin() + (begin + 1), data.begin() + i);
            begin = i;
        }
    }
    dataList.emplace_back(data.begin() + (begin + 1), data.begin() + i);

    return dataList;
}

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
    qls::Session session(network);

    std::cout << "Connecting to server...\n";
    while (true)
    {
        if (!can_be_used)
        {
            using namespace std::chrono;
            std::this_thread::sleep_for(0.1s);
            continue;
        }
        char buffer[8192]{0};
        std::string command;
        std::cin.getline(buffer, sizeof(buffer) - 1);
        command = buffer;
        if (command == "") continue;
        if (command == "stop")
        {
            return 0;
        }

        std::vector<std::string> vec = split(strip(command));
        try
        {
            if (vec[0] == "registerUser")
            {
                opt::Option opt;
                opt.add("email", opt::Option::OptionType::OPT_REQUIRED);
                opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
                opt.parse(std::vector<std::string>{vec.begin() + 1, vec.end()});
                qls::UserID user_id;
                if (session.registerUser(opt.get_string("email"), opt.get_string("password"), user_id))
                    std::cout << "Successfully created a new user! User id is: " << user_id << '\n';
                else
                    std::cout << "Failed to created a new user!\n";
            }
            else if (vec[0] == "loginUser")
            {
                opt::Option opt;
                opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
                opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
                opt.parse(std::vector<std::string>{vec.begin() + 1, vec.end()});
                qls::UserID user_id;
                if (session.loginUser(qls::UserID(opt.get_int("userid")), opt.get_string("password")))
                    std::cout << "Successfully logined a user!\n";
                else
                    std::cout << "Failed to logined a user!\n";
            }
            else
            {
                std::cout << "Could not find a command: " << vec[0] << '\n';
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

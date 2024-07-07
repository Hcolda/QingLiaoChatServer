#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <memory>

#include <SQLProcess.hpp>

namespace qls
{
    class DataManager
    {
    public:
        DataManager() = default;
        ~DataManager() = default;

        void init();

        /// @brief add user
        /// @param user_id 
        /// @param pwd_hash hash of password
        void addNewUser(long long user_id, size_t pwd_hash);

        /// @brief change user password
        /// @param user_id 
        /// @param pwd_hash hash of password
        void changeUserPassword(long long user_id, size_t pwd_hash);

        /// @brief 
        /// @param user_id 
        /// @param pwd_hash hash of password
        /// @return true if verified successfully
        bool verifyUserPassword(long long user_id, size_t pwd_hash);
    };
}

#endif // !DATA_MANAGER_H
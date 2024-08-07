#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <memory>
#include <SQLProcess.hpp>

namespace qls
{
    /**
     * @class DataManager
     * @brief Manages user data and interactions with the database.
     */
    class DataManager
    {
    public:
        DataManager() = default;
        ~DataManager() = default;

        /**
         * @brief Initializes the data manager.
         */
        void init();

        /**
         * @brief Adds a new user to the database.
         * @param user_id The ID of the user.
         * @param pwd_hash The hash of the user's password.
         */
        void addNewUser(long long user_id, size_t pwd_hash);

        /**
         * @brief Changes the password of an existing user.
         * @param user_id The ID of the user.
         * @param pwd_hash The new hash of the user's password.
         */
        void changeUserPassword(long long user_id, size_t pwd_hash);

        /**
         * @brief Verifies a user's password.
         * @param user_id The ID of the user.
         * @param pwd_hash The hash of the user's password.
         * @return True if the password is verified successfully, false otherwise.
         */
        bool verifyUserPassword(long long user_id, size_t pwd_hash);
    };
}

#endif // !DATA_MANAGER_H

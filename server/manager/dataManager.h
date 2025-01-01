#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <memory>
#include <SQLProcess.hpp>

#include "userid.hpp"

namespace qls
{
    
/**
 * @class DataManager
 * @brief Manages user data and interactions with the database.
 */
class DataManager final
{
public:
    DataManager() = default;
    DataManager(const DataManager&) = delete;
    DataManager(DataManager&&) = delete;
    ~DataManager() = default;

    DataManager& operator=(const DataManager&) = delete;
    DataManager& operator=(DataManager&&) = delete;

    /**
     * @brief Initializes the data manager.
     */
    void init();

    /**
     * @brief Adds a new user to the database.
     * @param user_id The ID of the user.
     * @param pwd_hash The hash of the user's password.
     */
    void addNewUser(UserID user_id, std::size_t pwd_hash);

    /**
     * @brief Changes the password of an existing user.
     * @param user_id The ID of the user.
     * @param pwd_hash The new hash of the user's password.
     */
    void changeUserPassword(UserID user_id, std::size_t pwd_hash);

    /**
     * @brief Verifies a user's password.
     * @param user_id The ID of the user.
     * @param pwd_hash The hash of the user's password.
     * @return True if the password is verified successfully, false otherwise.
     */
    bool verifyUserPassword(UserID user_id, std::size_t pwd_hash);
};

} // namespace qls

#endif // !DATA_MANAGER_H

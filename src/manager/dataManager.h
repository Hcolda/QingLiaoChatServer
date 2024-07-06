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

        /// @brief It must be set before run the data manager
        /// @param process SQLDBProcess
        void setSQLDBProcess(std::shared_ptr<quqisql::SQLDBProcess> process);

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

    private:
        std::shared_ptr<quqisql::SQLDBProcess> m_sql_process;
    };
}

#endif // !DATA_MANAGER_H
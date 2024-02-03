#ifndef SQL_PROCESS_HPP
#define SQL_PROCESS_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <memory>
#include <format>
#include <conncpp.hpp>

namespace quqisql
{
    class SQLDBProcess
    {
    public:
        SQLDBProcess() :
            m_port(0) {}

        SQLDBProcess(const std::string& username,
                     const std::string& password,
                     const std::string& database_name,
                     const std::string& host,
                     unsigned short port
        )
        {
            m_username = username;
            m_password = password;
            m_database_name = database_name;
            m_host = host;
            m_port = port;
        }

        SQLDBProcess(const SQLDBProcess&) = delete;
        SQLDBProcess(SQLDBProcess&) = delete;
        ~SQLDBProcess() = default;

        void setSQLServerInfo(const std::string& username,
                              const std::string& password,
                              const std::string& database_name,
                              const std::string& host,
                              unsigned short port
                              )
        {
            m_username = username;
            m_password = password;
            m_database_name = database_name;
            m_host = host;
            m_port = port;
        }

        void connectSQLServer()
        {
            sql::Driver* driver = sql::mariadb::get_driver_instance();
            sql::Properties properties({
                {"user", m_username},
                {"password", m_password}
                });

            sql::SQLString url(std::format("jdbc:mariadb://{}:{}/{}", m_host, m_port, m_database_name).c_str());
            m_sqlconnection = std::shared_ptr<sql::Connection>(
                driver->connect(url, properties),
                [](sql::Connection* conn) {conn->close(); });
        }

        std::shared_ptr<sql::ResultSet> executeQuery(const std::string& command)
        {
            if (m_sqlconnection.get() == nullptr)
                throw std::runtime_error("connection is null");

            std::shared_ptr<sql::Statement> statement(m_sqlconnection->createStatement());

            // Execute query
            return std::shared_ptr<sql::ResultSet>{statement->executeQuery(command),
                [statement](sql::ResultSet* set){set->last(); statement->close();}};
        }

        void executeUpdate(const std::string& command)
        {
            if (m_sqlconnection.get() == nullptr)
                throw std::runtime_error("connection is null");

            std::shared_ptr<sql::Statement> statement(m_sqlconnection->createStatement());

            statement->executeUpdate(command);
        }
    private:
        std::string     m_username;
        std::string     m_password;
        std::string     m_database_name;
        std::string     m_host;
        unsigned short  m_port;

        std::shared_ptr<sql::Connection>    m_sqlconnection;
    };
}

#endif // !SQL_PROCESS_HPP

#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <memory>
#include <format>
#include <mariadb/conncpp.hpp>

namespace qls
{
    class SQLDBProcess
    {
    public:
        SQLDBProcess() = default;
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

        bool connectSQLServer()
        {
            try
            {
                sql::Driver* driver = sql::mariadb::get_driver_instance();
                sql::Properties properties({
                    {"user", m_username},
                    {"password", m_password}
                    });

                sql::SQLString url(std::format("jdbc:mariadb://{}:{}/{}", m_host, m_port, m_database_name));
                m_sqlconnection = std::shared_ptr<sql::Connection>(driver->connect(url, properties));
            }
            catch (...)
            {
                return false;
            }
            return true;
        }

        template<typename... Args>
        std::unique_ptr<sql::ResultSet> executeCommand(const std::string& command, Args... args)
        {
            if (m_sqlconnection.get() == nullptr)
                throw std::runtime_error("connection is null");

            size_t position = 1;
            std::shared_ptr<sql::Statement> statement(m_sqlconnection->createStatement());

            std::unique_ptr<sql::PreparedStatement> preparedStatement(m_sqlconnection->prepareStatement(command));

            // Bind values to SQL statement
            (set_parameter(args, preparedStatement, position++), ...);

            // Execute query
            return std::unique_ptr<sql::ResultSet>{preparedStatement->executeQuery()};
        }

    protected:
        enum class parameter_type_n
        {
            Unknown = 0,
            Int,
            Short,
            Int64,
            UInt,
            UInt64,
            Float,
            Double,
            String,
            Boolean
        };

        template<parameter_type_n Type>
        struct const_parameter_type
        {
            static constexpr parameter_type_n value = Type;

            using type = const_parameter_type;

            constexpr operator parameter_type_n() const noexcept
            {
                return value;
            }

            constexpr parameter_type_n operator()() const noexcept
            {
                return value;
            }
        };

        template<class Ty>
        struct parameter_type {
            static constexpr parameter_type_n value = parameter_type_n::Unknown;
        };

        template<>
        struct parameter_type<int> : const_parameter_type<parameter_type_n::Int> {};

        template<>
        struct parameter_type<long> : const_parameter_type<parameter_type_n::Int> {};

        template<>
        struct parameter_type<short> : const_parameter_type<parameter_type_n::Short> {};

        template<>
        struct parameter_type<long long> : const_parameter_type<parameter_type_n::Int64> {};

        template<>
        struct parameter_type<unsigned int> : const_parameter_type<parameter_type_n::UInt> {};

        template<>
        struct parameter_type<size_t> : const_parameter_type<parameter_type_n::UInt64> {};

        template<>
        struct parameter_type<float> : const_parameter_type<parameter_type_n::Float> {};

        template<>
        struct parameter_type<double> : const_parameter_type<parameter_type_n::Double> {};

        template<>
        struct parameter_type<bool> : const_parameter_type<parameter_type_n::Boolean> {};

        template<>
        struct parameter_type<std::string> : const_parameter_type<parameter_type_n::String> {};

        template<>
        struct parameter_type<const char*> : const_parameter_type<parameter_type_n::String> {};

        template<typename Ty>
        static void set_parameter(const Ty& value, std::unique_ptr<sql::PreparedStatement>& pp, size_t pos)
        {
            switch (parameter_type<Ty>::value)
            {
            case qls::SQLDBProcess::parameter_type_n::Unknown:
                throw std::logic_error("unknown type");
                break;
            case qls::SQLDBProcess::parameter_type_n::Int:
                pp->setInt(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::Short:
                pp->setShort(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::Int64:
                pp->setInt64(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::UInt:
                pp->setUInt(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::UInt64:
                pp->setUInt64(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::Float:
                pp->setFloat(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::Double:
                pp->setDouble(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::String:
                pp->setString(pos, value);
                break;
            case qls::SQLDBProcess::parameter_type_n::Boolean:
                pp->setBoolean(pos, value);
                break;
            default:
                break;
            }
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

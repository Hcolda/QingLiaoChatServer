#ifndef SQL_PROCESS_HPP
#define SQL_PROCESS_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <memory>
#include <format>
#include <conncpp.hpp>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <queue>
#include <functional>
#include <future>
#include <utility>

namespace quqisql
{
    class SQLDBProcess
    {
    public:
        SQLDBProcess() :
            m_port(-1),
            m_thread_is_running(false) {}

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
        ~SQLDBProcess()
        {
            m_thread_is_running = false;
            m_cv.notify_all();
            if (m_work_thread.joinable())
                m_work_thread.join();
        }

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
            if (this->m_port == -1 || this->m_port > 65535)
                throw std::logic_error("Data hasn't been initialized!");
            if (this->m_thread_is_running)
                throw std::logic_error("You have connected the server!");

            sql::Driver* driver = sql::mariadb::get_driver_instance();
            sql::Properties properties({
                {"user", m_username},
                {"password", m_password}
                });

            sql::SQLString url(std::format("jdbc:mariadb://{}:{}/{}",
                m_host, m_port, m_database_name).c_str());
            m_sqlconnection = std::shared_ptr<sql::Connection>(
                driver->connect(url, properties),
                [](sql::Connection* conn) {conn->close(); });

            this->m_thread_is_running = true;
            this->m_work_thread = std::thread([this]() -> void {
                while (m_thread_is_running)
                {
                    std::unique_lock<std::mutex> lock(m_function_queue_mutex);
                    this->m_cv.wait(lock,
                        [this](){ return !m_function_queue.empty() || !m_thread_is_running; });

                    if (!m_thread_is_running) return;
                    if (m_function_queue.empty())
                        continue;

                    auto func = std::move(m_function_queue.front());
                    m_function_queue.pop();
                    lock.unlock();

                    func();
                }
                });
        }

        template<typename Func, typename... Args>
        auto submit(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
        {
            auto package = std::make_shared<std::packaged_task<decltype(func(args...))(Args...)>>(std::forward<Func>(func));

            std::unique_lock<std::mutex> lock(m_function_queue_mutex);
            m_function_queue.push(std::bind([package](auto&&... args) -> void {
                (*package)(std::forward<decltype(args)>(args)...);
                }, std::forward<Args>(args)...));
            lock.unlock();
            m_cv.notify_all();

            return package->get_future();
        }

        std::shared_ptr<sql::ResultSet> executeQuery(const std::string& command)
        {
            if (!m_sqlconnection)
                throw std::runtime_error("Connection is null");

            std::shared_ptr<sql::Statement> statement(m_sqlconnection->createStatement());

            // Execute query
            return std::shared_ptr<sql::ResultSet>{statement->executeQuery(command),
                [statement](sql::ResultSet* set){set->last(); statement->close();}};
        }

        void executeUpdate(const std::string& command)
        {
            if (!m_sqlconnection)
                throw std::runtime_error("Connection is null");

            std::shared_ptr<sql::Statement> statement(m_sqlconnection->createStatement());

            statement->executeUpdate(command);
        }
        
    private:
        std::string     m_username;
        std::string     m_password;
        std::string     m_database_name;
        std::string     m_host;
        int             m_port;

        std::shared_ptr<sql::Connection>    m_sqlconnection;

        std::thread                         m_work_thread;
        std::queue<std::function<void()>>   m_function_queue;
        std::mutex                          m_function_queue_mutex;
        std::atomic<bool>                   m_thread_is_running;
        std::condition_variable             m_cv;
    };
}

#endif // !SQL_PROCESS_HPP

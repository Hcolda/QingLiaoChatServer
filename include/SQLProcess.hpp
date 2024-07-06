#ifndef SQL_PROCESS_HPP
#define SQL_PROCESS_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <memory>
#include <format>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <queue>
#include <functional>
#include <future>
#include <utility>

#include <asio.hpp>
#include <conncpp.hpp>

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
            m_thread_is_running_2 = false;
            m_cv.notify_all();
            m_cv_2.notify_all();
            if (m_work_thread.joinable())
                m_work_thread.join();
            if (m_work_thread_2.joinable())
                m_work_thread_2.join();
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

            this->m_thread_is_running_2 = true;
            this->m_work_thread_2 = std::thread([this]() -> void {
                while (m_thread_is_running_2)
                {
                    std::unique_lock<std::mutex> lock(m_function_queue_mutex_2);
                    this->m_cv_2.wait(lock,
                        [this](){ return !m_function_queue_2.empty() || !m_thread_is_running_2; });

                    if (!m_thread_is_running_2) return;
                    if (m_function_queue_2.empty())
                        continue;

                    auto func = std::move(m_function_queue_2.front());
                    m_function_queue_2.pop();
                    lock.unlock();

                    func();
                }
                });
        }

        template<typename Func, typename... Args>
        auto submit(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
        {
            auto package = std::make_shared<std::packaged_task<decltype(func(args...))(Args...)>>(std::forward<Func>(func));

            {
                std::unique_lock<std::mutex> lock(m_function_queue_mutex);
                m_function_queue.push(std::bind([package](auto&&... args) -> void {
                    (*package)(std::forward<decltype(args)>(args)...);
                    }, std::forward<Args>(args)...));
            }
            m_cv.notify_all();

            return package->get_future();
        }

        template<typename Func, typename... Args>
        auto submit_2(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
        {
            auto package = std::make_shared<std::packaged_task<decltype(func(args...))(Args...)>>(std::forward<Func>(func));

            {
                std::unique_lock<std::mutex> lock(m_function_queue_mutex_2);
                m_function_queue_2.push(std::bind([package](auto&&... args) -> void {
                    (*package)(std::forward<decltype(args)>(args)...);
                    }, std::forward<Args>(args)...));
            }
            m_cv.notify_all();

            return package->get_future();
        }

        template<typename R, typename Func, typename... Args,
            asio::completion_token_for<void(R)> CompletionToken>
        auto awaitableSubmit(CompletionToken&& token, Func&& func, Args&&... args)
        {
            // 还有更好的方法，此处还没有实现更好的理想方法
            // auto init = [this](auto completion_handler, auto&& func, auto&&... args) mutable -> void {
            //     std::future<R> future = this->submit(std::forward<decltype(func)>(func), std::forward<decltype(args)>(args)...);
            //     std::thread([](auto completion_handler, std::future<R> future) {
            //         std::move(completion_handler)(future.get());
            //         }, std::move(completion_handler), std::move(future)).detach();
            //     };

            // return asio::async_initiate<CompletionToken, void(R)>(
            //     init, token, std::forward<Func>(func), std::forward<Args>(args)...);

            // 理想方法 但是是伪代码
            auto init = [this](auto completion_handler, auto&& func, auto&&... args) mutable -> void {
                std::future<R> future = this->submit(std::forward<decltype(func)>(func), std::forward<decltype(args)>(args)...);
                this->submit_2([](auto completion_handler, std::future<R> future) {
                    std::move(completion_handler)(future.get());
                    }, std::move(completion_handler), std::move(future));
                };

            return asio::async_initiate<CompletionToken, void(R)>(
                init, token, std::forward<Func>(func), std::forward<Args>(args)...);
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

        void preparedUpdate(const std::string& preparedCommand,
            std::function<void(std::shared_ptr<sql::PreparedStatement>&)> callback)
        {
            if (!m_sqlconnection)
                throw std::runtime_error("Connection is null");

            std::shared_ptr<sql::PreparedStatement> stmnt(
                m_sqlconnection->prepareStatement(preparedCommand));
            callback(stmnt);

            stmnt->executeUpdate();
        }

        std::shared_ptr<sql::ResultSet> preparedQuery(const std::string& preparedCommand,
            std::function<void(std::shared_ptr<sql::PreparedStatement>&)> callback)
        {
            if (!m_sqlconnection)
                throw std::runtime_error("Connection is null");

            std::shared_ptr<sql::PreparedStatement> stmnt(
                m_sqlconnection->prepareStatement(preparedCommand));
            callback(stmnt);
            
            return std::shared_ptr<sql::ResultSet>{stmnt->executeQuery(),
                [stmnt](sql::ResultSet* set) {set->last(); stmnt->close(); }};
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

        std::thread                         m_work_thread_2;
        std::queue<std::function<void()>>   m_function_queue_2;
        std::mutex                          m_function_queue_mutex_2;
        std::atomic<bool>                   m_thread_is_running_2;
        std::condition_variable             m_cv_2;
    };
}

#endif // !SQL_PROCESS_HPP

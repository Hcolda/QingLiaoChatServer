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
#include <mariadb/conncpp.hpp>

namespace qls
{

/**
 * @class SQLDBProcess
 * @brief Handles SQL database processes including connection, query execution, and task management.
 */
class SQLDBProcess final
{
public:
    /**
     * @brief Default constructor.
     */
    SQLDBProcess() :
        m_port(-1),
        m_thread_is_running(false) {}

    /**
     * @brief Parameterized constructor.
     * @param username Database username.
     * @param password Database password.
     * @param database_name Database name.
     * @param host Database host address.
     * @param port Database port.
     */
    SQLDBProcess(std::string_view username,
                    std::string_view password,
                    std::string_view database_name,
                    std::string_view host,
                    unsigned short port)
    {
        m_username = username;
        m_password = password;
        m_database_name = database_name;
        m_host = host;
        m_port = port;
    }

    // Delete copy constructor and assignment operator
    SQLDBProcess(const SQLDBProcess&) = delete;
    SQLDBProcess(SQLDBProcess&&) = delete;
    SQLDBProcess& operator=(const SQLDBProcess&) = delete;
    SQLDBProcess& operator=(SQLDBProcess&&) = delete;

    /**
     * @brief Destructor.
     */
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

    /**
     * @brief Sets SQL server information.
     * @param username Database username.
     * @param password Database password.
     * @param database_name Database name.
     * @param host Database host address.
     * @param port Database port.
     */
    void setSQLServerInfo(std::string_view username,
                            std::string_view password,
                            std::string_view database_name,
                            std::string_view host,
                            unsigned short port)
    {
        m_username = username;
        m_password = password;
        m_database_name = database_name;
        m_host = host;
        m_port = port;
    }

    /**
     * @brief Connects to the SQL server and starts the worker threads.
     */
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
            [](sql::Connection* conn) { conn->close(); });

        this->m_thread_is_running = true;
        this->m_work_thread = std::thread([this]() -> void {
            while (m_thread_is_running) {
                std::unique_lock<std::mutex> lock(m_function_queue_mutex);
                this->m_cv.wait(lock,
                    [this]() { return !m_function_queue.empty() || !m_thread_is_running; });

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
            while (m_thread_is_running_2) {
                std::unique_lock<std::mutex> lock(m_function_queue_mutex_2);
                this->m_cv_2.wait(lock,
                    [this]() { return !m_function_queue_2.empty() || !m_thread_is_running_2; });

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

    /**
     * @brief Submits a function to the task queue and returns a future to get the result.
     * @tparam Func Function type.
     * @tparam Args Argument types.
     * @param func Function to submit.
     * @param args Arguments to pass to the function.
     * @return std::future to get the result of the function.
     */
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

    /**
     * @brief Submits a function to the secondary task queue and returns a future to get the result.
     * @tparam Func Function type.
     * @tparam Args Argument types.
     * @param func Function to submit.
     * @param args Arguments to pass to the function.
     * @return std::future to get the result of the function.
     */
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

    /**
     * @brief Submits a function as an awaitable task.
     * @tparam R Return type.
     * @tparam Func Function type.
     * @tparam Args Argument types.
     * @tparam CompletionToken Type of the completion token.
     * @param token Completion token.
     * @param func Function to submit.
     * @param args Arguments to pass to the function.
     * @return An awaitable object.
     */
    template<typename R, typename Func, typename... Args,
        asio::completion_token_for<void(R)> CompletionToken>
    auto awaitableSubmit(CompletionToken&& token, Func&& func, Args&&... args)
    {
        // Ideal method but not implemented
        auto init = [this](auto completion_handler, auto&& func, auto&&... args) mutable -> void {
            std::future<R> future = this->submit(std::forward<decltype(func)>(func), std::forward<decltype(args)>(args)...);
            this->submit_2([](auto completion_handler, std::future<R> future) {
                std::move(completion_handler)(future.get());
                }, std::move(completion_handler), std::move(future));
            };

        return asio::async_initiate<CompletionToken, void(R)>(
            init, token, std::forward<Func>(func), std::forward<Args>(args)...);
    }

    /**
     * @brief Executes an SQL query and returns the result set.
     * @param command SQL query string.
     * @return std::shared_ptr<sql::ResultSet> containing the query result.
     * @throws std::runtime_error if the connection is null.
     */
    std::shared_ptr<sql::ResultSet> executeQuery(const std::string& command)
    {
        if (!m_sqlconnection)
            throw std::runtime_error("Connection is null");

        std::shared_ptr<sql::Statement> statement(m_sqlconnection->createStatement());

        // Execute query
        return std::shared_ptr<sql::ResultSet>{statement->executeQuery(command),
            [statement](sql::ResultSet* set) { set->last(); statement->close(); }};
    }

    /**
     * @brief Executes an SQL update command.
     * @param command SQL update string.
     * @throws std::runtime_error if the connection is null.
     */
    void executeUpdate(const std::string& command)
    {
        if (!m_sqlconnection)
            throw std::runtime_error("Connection is null");

        std::shared_ptr<sql::Statement> statement(m_sqlconnection->createStatement());

        statement->executeUpdate(command);
    }

    /**
     * @brief Executes a prepared SQL update command with a callback.
     * @param preparedCommand Prepared SQL update string.
     * @param callback Callback to set the prepared statement parameters.
     * @throws std::runtime_error if the connection is null.
     */
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

    /**
     * @brief Executes a prepared SQL query command with a callback and returns the result set.
     * @param preparedCommand Prepared SQL query string.
     * @param callback Callback to set the prepared statement parameters.
     * @return std::shared_ptr<sql::ResultSet> containing the query result.
     * @throws std::runtime_error if the connection is null.
     */
    std::shared_ptr<sql::ResultSet> preparedQuery(const std::string& preparedCommand,
        std::function<void(std::shared_ptr<sql::PreparedStatement>&)> callback)
    {
        if (!m_sqlconnection)
            throw std::runtime_error("Connection is null");

        std::shared_ptr<sql::PreparedStatement> stmnt(
            m_sqlconnection->prepareStatement(preparedCommand));
        callback(stmnt);

        return std::shared_ptr<sql::ResultSet>{stmnt->executeQuery(),
            [stmnt](sql::ResultSet* set) { set->last(); stmnt->close(); }};
    }

private:
    std::string     m_username; ///< Database username.
    std::string     m_password; ///< Database password.
    std::string     m_database_name; ///< Database name.
    std::string     m_host; ///< Database host address.
    int             m_port; ///< Database port.

    std::shared_ptr<sql::Connection>    m_sqlconnection; ///< SQL connection.

    std::thread                         m_work_thread; ///< Main worker thread.
    std::queue<std::function<void()>>   m_function_queue; ///< Task queue for main worker thread.
    std::mutex                          m_function_queue_mutex; ///< Mutex for main task queue.
    std::atomic<bool>                   m_thread_is_running; ///< Flag indicating if main worker thread is running.
    std::condition_variable             m_cv; ///< Condition variable for main task queue.

    std::thread                         m_work_thread_2; ///< Secondary worker thread.
    std::queue<std::function<void()>>   m_function_queue_2; ///< Task queue for secondary worker thread.
    std::mutex                          m_function_queue_mutex_2; ///< Mutex for secondary task queue.
    std::atomic<bool>                   m_thread_is_running_2; ///< Flag indicating if secondary worker thread is running.
    std::condition_variable             m_cv_2; ///< Condition variable for secondary task queue.
};

} // namespace qls

#endif // !SQL_PROCESS_HPP

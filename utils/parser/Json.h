#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <fstream>
#include <sstream>
#include <memory>

namespace qjson
{
    /**
     * @brief Enumeration for JSON value types.
     */
    enum JValueType
    {
        JNull,
        JInt,
        JDouble,
        JBool,
        JString,
        JList,
        JDict
    };

    class JObject;

    using null_t = bool;
    using int_t = long long;
    using bool_t = bool;
    using double_t = long double;
    using string_t = std::string;
    using list_t = std::vector<JObject>;
    using dict_t = std::unordered_map<std::string, JObject>;
    using value_t = std::variant<int_t, bool_t, double_t, string_t, list_t, dict_t>;

    /**
     * @brief Class representing a JSON object.
     */
    class JObject
    {
    public:
        JObject();
        JObject(const JObject& jo);
        JObject(JObject&& jo) noexcept;
        JObject(JValueType jvt);
        JObject(long long value);
        JObject(long value);
        JObject(int value);
        JObject(short value);
        JObject(bool value);
        JObject(long double value);
        JObject(double value);
        JObject(float value);
        JObject(const char* data);
        JObject(const std::string& data);
        JObject(std::string_view data);
        JObject(std::string&& data) noexcept;
        ~JObject();

        JObject& operator=(const JObject& jo);
        JObject& operator=(JObject&& jo) noexcept;

        friend bool operator==(const JObject& joa, const JObject& jo);
        friend bool operator==(const JObject& jo, JValueType type);

        const JObject& operator[](std::size_t itor) const;
        JObject& operator[](std::size_t itor);
        const JObject& operator[](int itor) const;
        JObject& operator[](int itor);
        const JObject& operator[](const char* str) const;
        JObject& operator[](const char* str);

        void push_back(const JObject& jo);
        void push_back(JObject&& jo);
        void pop_back();
        bool hasMember(const std::string& str) const;

        JValueType getType() const noexcept;
        const list_t& getList() const;
        list_t& getList();
        const dict_t& getDict() const;
        dict_t& getDict();
        const long long& getInt() const;
        long long& getInt();
        const long double& getDouble() const;
        long double& getDouble();
        const bool& getBool() const;
        bool& getBool();
        const std::string& getString() const;
        std::string& getString();

    private:
        std::unique_ptr<value_t> m_value; ///< The value of the JSON object.
        JValueType m_type; ///< The type of the JSON value.
    };

    /**
     * @brief Class for parsing JSON data.
     */
    class JParser
    {
    public:
        JParser() = default;

        /**
         * @brief Parses JSON data from a string view.
         * @param data The JSON data to parse.
         * @return The parsed JSON object.
         */
        JObject parse(std::string_view data);

        /**
         * @brief Quickly parses JSON data from an input file stream.
         * @param infile The input file stream.
         * @return The parsed JSON object.
         */
        static JObject fastParse(std::ifstream& infile);

        /**
         * @brief Quickly parses JSON data from a string view.
         * @param data The JSON data to parse.
         * @return The parsed JSON object.
         */
        static JObject fastParse(std::string_view data);

    protected:
        JObject parse_(std::string_view data, std::size_t data_size, std::size_t& itor);
        void skipSpace(std::string_view data, std::size_t data_size, std::size_t& itor, long long& error_line);
        std::string getString(std::string_view data, std::size_t data_size, std::size_t& itor, long long error_line);
        JObject getNumber(std::string_view data, std::size_t data_size, std::size_t& itor, long long error_line);
        JObject getBool(std::string_view data, std::size_t data_size, std::size_t& itor, long long error_line);
        JObject getNull(std::string_view data, std::size_t data_size, std::size_t& itor, long long error_line);
        std::string getLogicErrorString(long long error_line);
        std::string getLogicErrorString(long long error_line, std::string_view error);
    };

    /**
     * @brief Class for writing JSON data.
     */
    class JWriter
    {
    public:
        JWriter() = default;
        ~JWriter() = default;

        /**
         * @brief Writes a JSON object to a string.
         * @param jo The JSON object to write.
         * @return The JSON data as a string.
         */
        std::string write(const JObject& jo);

        /**
         * @brief Writes a formatted JSON object to a string.
         * @param jo The JSON object to write.
         * @param n The indentation level.
         * @return The formatted JSON data as a string.
         */
        std::string formatWrite(const JObject& jo, std::size_t indent = 4, std::size_t n = 1);

        /**
         * @brief Quickly writes a JSON object to a string.
         * @param jo The JSON object to write.
         * @return The JSON data as a string.
         */
        static std::string fastWrite(const JObject& jo);

        /**
         * @brief Quickly writes a formatted JSON object to a string.
         * @param jo The JSON object to write.
         * @return The formatted JSON data as a string.
         */
        static std::string fastFormatWrite(const JObject& jo, std::size_t indent = 4);
    };
}

#endif // !JSON_HPP

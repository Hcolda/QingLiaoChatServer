#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <fstream>
#include <sstream>
#include <format>

namespace qjson
{
	enum class JValueType
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

		JObject(std::string&& data);

		~JObject();

		JObject& operator =(const JObject& jo);

		JObject& operator =(JObject&& jo) noexcept;

		friend bool operator ==(const JObject& joa, const JObject& jo);

		friend bool operator ==(const JObject& jo, JValueType type);

		const JObject& operator[](size_t itor) const;

		JObject& operator[](size_t itor);

		const JObject& operator[](int itor) const;

		JObject& operator[](int itor);

		const JObject& operator[](const char* str) const;

		JObject& operator[](const char* str);

		void push_back(const JObject& jo);

		void push_back(JObject&& jo);

		void pop_back();

		bool hasMember(const std::string& str) const;

		JValueType getType() const;

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

	public:
		using value_t = std::variant<int_t, bool_t, double_t, string_t, list_t, dict_t>;
	private:
		mutable value_t* m_value;
		mutable JValueType m_type;
	};

	class JParser
	{
	public:
		JParser() = default;

		JObject parse(std::string_view data);

		static JObject fastParse(std::ifstream& infile);

		static JObject fastParse(const std::string_view data);

	protected:
		JObject _parse(std::string_view data, size_t& itor);

		void skipSpace(std::string_view data, size_t& itor, long long& error_line);

		std::string getString(std::string_view data, size_t& itor, long long error_line);

		JObject getNumber(std::string_view data, size_t& itor, long long error_line);

		JObject getBool(std::string_view data, size_t& itor, long long error_line);

		JObject getNull(std::string_view data, size_t& itor, long long error_line);

		std::string get_logic_error_string(long long error_line);
	};

	class JWriter
	{
	public:
		JWriter() = default;
		~JWriter() = default;

		std::string write(const JObject& jo);

		std::string formatWrite(const JObject& jo, size_t n = 1);

		static std::string fastWrite(const JObject& jo)
		{
			static JWriter jw;
			return std::move(jw.write(jo) + '\n');
		}

		static std::string fastFormatWrite(const JObject& jo)
		{
			static JWriter jw;
			return std::move(jw.formatWrite(jo) + '\n');
		}
	};
}

#endif // !JSON_HPP

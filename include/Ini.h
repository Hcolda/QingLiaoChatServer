#ifndef INI_HPP
#define INI_HPP

#include <unordered_map>
#include <string>
#include <string_view>
#include <fstream>
#include <format>
#include <stdexcept>

namespace qini
{
	class INIParser;
	class INIWriter;

	class INIObject
	{
	public:
		class Section
		{
		public:
			class iterator
			{
			public:
				iterator(const std::unordered_map<std::string, std::string>::iterator& itor);

				iterator(std::unordered_map<std::string, std::string>::iterator&& itor);

				iterator& operator ++();

				std::string operator *();

				friend bool operator ==(const iterator& a, const iterator& b);

				friend bool operator !=(const iterator& a, const iterator& b);

			private:
				std::unordered_map<std::string, std::string>::iterator m_itor;
			};

			Section(std::unordered_map<std::string, std::string>& section);

			Section(const Section&) = delete;
			Section(Section&&) = delete;

			Section& operator =(const Section&) = delete;
			Section& operator =(Section&&) = delete;

			/*
			* @brief 获取键值
			* @param keyName 节名
			* @return std::string& 键对应的值
			*/
			std::string& operator [](const std::string& keyName);

			/*
			* @brief 获取键值
			* @param keyName 节名
			* @return std::string& 键对应的值
			*/
			const std::string& operator [](const std::string& keyName) const;

			iterator begin();

			iterator end();

		private:
			std::unordered_map<std::string, std::string>& m_keys;
		};

		class Const_Section
		{
		public:
			class const_iterator
			{
			public:
				const_iterator(const std::unordered_map<std::string, std::string>::const_iterator& itor);

				const_iterator(std::unordered_map<std::string, std::string>::const_iterator&& itor);

				const_iterator& operator ++();

				std::string operator *();

				friend bool operator ==(const const_iterator& a, const const_iterator& b);

				friend bool operator !=(const const_iterator& a, const const_iterator& b);

			private:
				std::unordered_map<std::string, std::string>::const_iterator m_itor;
			};

			Const_Section(const std::unordered_map<std::string, std::string>& section);

			Const_Section(const Section&) = delete;
			Const_Section(Section&&) = delete;

			Const_Section& operator =(const Section&) = delete;
			Const_Section& operator =(Section&&) = delete;

			/*
			* @brief 获取键值
			* @param keyName 节名
			* @return std::string& 键对应的值
			*/
			const std::string& operator [](const std::string& keyName) const;

			const_iterator begin();

			const_iterator end();

		private:
			const std::unordered_map<std::string, std::string>& m_keys;
		};

		class iterator
		{
		public:
			iterator(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator& itor);

			iterator(std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator&& itor);

			iterator& operator ++();

			Section operator *();

			friend bool operator ==(const iterator& a, const iterator& b);

			friend bool operator !=(const iterator& a, const iterator& b);

		private:
			std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator m_itor;
		};

		class const_iterator
		{
		public:
			const_iterator(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::const_iterator& itor);

			const_iterator(std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::const_iterator&& itor);

			const_iterator& operator ++();

			Const_Section operator *();

			friend bool operator ==(const const_iterator& a, const const_iterator& b);

			friend bool operator !=(const const_iterator& a, const const_iterator& b);

		private:
			std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::const_iterator m_itor;
		};

		INIObject() = default;

		INIObject(const INIObject& ob);

		INIObject(INIObject&& ob) noexcept;

		~INIObject() = default;

		INIObject& operator =(const INIObject& ob);

		INIObject& operator =(INIObject&& ob) noexcept;

		/*
		* @brief 获取节值
		* @param sectionName 节名
		* @return Section 节类
		*/
		Section operator [](const std::string& sectionName);

		Const_Section operator [](const std::string& sectionName) const;

		iterator begin();

		iterator end();

		friend bool operator ==(const INIObject& ia, const INIObject& ib);

		friend bool operator !=(const INIObject& ia, const INIObject& ib);

	private:
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_sections;

		friend class INIParser;
		friend class INIWriter;
	};

	class INIParser
	{
	public:
		INIParser() = default;
		~INIParser() = default;

		/*
		* @brief 解析数据
		* @param data 数据
		* @return INIObject ini数据
		*/
		INIObject parse(std::string_view data);

		/*
		* @brief 解析数据
		* @param data 数据
		* @return INIObject ini数据
		*/
		static INIObject fastParse(std::string_view data);

		static INIObject fastParse(std::ifstream& infile);

	protected:
		bool skipSpace(std::string_view::iterator& i, std::string_view data, long long& error_line);

		std::string getString(std::string_view::iterator& i, std::string_view data, long long error_line);

		std::string get_logic_error_string(long long error_line);
	};

	class INIWriter
	{
	public:
		INIWriter() = default;
		~INIWriter() = default;

		/*
		* @brief 写入数据
		* @param INIObject 数据
		* @return std::string 字符串数据
		*/
		std::string write(const INIObject& ob);

		/*
		* @brief 写入数据到文件
		* @param INIObject 数据
		* @param std::ofstream 文件流
		* @return true 成功
		* @return false 失败
		*/
		bool write(const INIObject& ob, std::ofstream& file);

		/*
		* @brief 写入数据
		* @param INIObject 数据
		* @return std::string 字符串数据
		*/
		static std::string fastWrite(const INIObject& ob);

		/*
		* @brief 写入数据到文件
		* @param INIObject 数据
		* @param std::ofstream 文件流
		* @return true 成功
		* @return false 失败
		*/
		static bool fastWrite(const INIObject& ob, std::ofstream& file);
	};
}

#endif // !INI_HPP

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
				iterator(const std::unordered_map<std::string, std::string>::iterator& itor) :
					m_itor(itor)
				{}

				iterator(std::unordered_map<std::string, std::string>::iterator&& itor) :
					m_itor(std::move(itor))
				{}

				iterator& operator ++()
				{
					m_itor++;
				}

				iterator& operator --()
				{
					m_itor--;
				}

				std::string operator *()
				{
					return std::move(m_itor->second);
				}

				friend bool operator ==(const iterator& a, const iterator& b)
				{
					return a.m_itor == b.m_itor;
				}

				friend bool operator !=(const iterator& a, const iterator& b)
				{
					return a.m_itor != b.m_itor;
				}

			private:
				std::unordered_map<std::string, std::string>::iterator m_itor;
			};

			Section(std::unordered_map<std::string, std::string>& section) :
				m_keys(section)
			{}

			Section(const Section&) = delete;
			Section(Section&&) = delete;

			Section& operator =(const Section&) = delete;
			Section& operator =(Section&&) = delete;

			/*
			* @brief 获取键值
			* @param keyName 节名
			* @return std::string& 键对应的值
			*/
			std::string& operator [](const std::string& keyName)
			{
				return m_keys[keyName];
			}

			/*
			* @brief 获取键值
			* @param keyName 节名
			* @return std::string& 键对应的值
			*/
			const std::string& operator [](const std::string& keyName) const
			{
				if (m_keys.find(keyName) == m_keys.end()) throw std::logic_error("Invalid Keyword");
				return m_keys[keyName];
			}

			iterator begin()
			{
				return { std::move(m_keys.begin()) };
			}

			iterator end()
			{
				return { std::move(m_keys.end()) };
			}

		private:
			std::unordered_map<std::string, std::string>& m_keys;
		};

		class iterator
		{
		public:
			iterator(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator& itor) :
				m_itor(itor)
			{}

			iterator(std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator&& itor) :
				m_itor(std::move(itor))
			{}

			iterator& operator ++()
			{
				m_itor++;
			}

			iterator& operator --()
			{
				m_itor--;
			}

			Section operator *()
			{
				return { m_itor->second };
			}

			friend bool operator ==(const iterator& a, const iterator& b)
			{
				return a.m_itor == b.m_itor;
			}

			friend bool operator !=(const iterator& a, const iterator& b)
			{
				return a.m_itor != b.m_itor;
			}

		private:
			std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator m_itor;
		};

		INIObject() = default;

		INIObject(const INIObject& ob) :
			m_sections(ob.m_sections)
		{}

		INIObject(INIObject&& ob) noexcept :
			m_sections(std::move(ob.m_sections))
		{}

		~INIObject() = default;

		INIObject& operator =(const INIObject& ob)
		{
			if (this == &ob)
				return *this;

			m_sections = ob.m_sections;
			return *this;
		}

		INIObject& operator =(INIObject&& ob) noexcept
		{
			if (this == &ob)
				return *this;

			m_sections = std::move(ob.m_sections);
			return *this;
		}

		/*
		* @brief 获取节值
		* @param sectionName 节名
		* @return Section 节类
		*/
		Section operator [](const std::string sectionName)
		{
			if (m_sections.find(sectionName) == m_sections.end()) throw std::logic_error("Invalid Section Name");
			return Section(m_sections[sectionName]);
		}

		iterator begin()
		{
			return { std::move(m_sections.begin()) };
		}

		iterator end()
		{
			return { std::move(m_sections.end()) };
		}

		friend bool operator ==(const INIObject& ia, const INIObject& ib)
		{
			return ia.m_sections == ib.m_sections;
		}

		friend bool operator !=(const INIObject& ia, const INIObject& ib)
		{
			return ia.m_sections != ib.m_sections;
		}

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

		/*INIObject parse(std::ifstream& file)
		{
			INIObject localObject;
		}*/

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

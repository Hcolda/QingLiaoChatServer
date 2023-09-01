#include "Ini.h"

#define INI_NAMESPACE_START namespace qini {
#define INI_NAMESPACE_END }

INI_NAMESPACE_START


INIObject INIParser::parse(std::string_view data)
{
	INIObject localObject;
	long long error_line = 0;

	std::string localSection;
	for (auto i = data.begin(); i != data.end(); i++)
	{
		if (!skipSpace(i, data, error_line))
			break;

		if (*i == '[')
		{
			i++;
			if (!skipSpace(i, data, error_line))
				throw std::logic_error(get_logic_error_string(error_line));

			localSection = getString(i, data, error_line);

			if (!skipSpace(i, data, error_line))
				throw std::logic_error(get_logic_error_string(error_line));

			if (*i == ']')
				i++;
			else
				throw std::logic_error(get_logic_error_string(error_line));
		}
		else if (*i == '=')
		{
			throw std::logic_error(get_logic_error_string(error_line));
		}
		else
		{
			if (localSection.empty())
				throw std::logic_error(get_logic_error_string(error_line));

			std::string localKey = getString(i, data, error_line);

			if (!skipSpace(i, data, error_line))
				throw std::logic_error(get_logic_error_string(error_line));

			if (*i == '=')
				i++;
			else
				throw std::logic_error(get_logic_error_string(error_line));

			if (!skipSpace(i, data, error_line))
				throw std::logic_error(get_logic_error_string(error_line));

			localObject.m_sections[localSection][localKey] = getString(i, data, error_line);
		}
	}

	return localObject;
}

INIObject qini::INIParser::fastParse(std::string_view data)
{
	static INIParser parser;
	return parser.parse(data);
}

bool INIParser::skipSpace(std::string_view::iterator& i, std::string_view data, long long& error_line)
{
	while (i != data.end() && (*i == ' ' || *i == '\n' || *i == '\t' || *i == ';' || *i == '#'))
	{
		if (*i == ';' || *i == '#')
		{
			for (; i != data.end() && *i != '\n'; i++) {}
		}
		else
		{
			if (*i == '\n') error_line++;
			i++;
		}
	}

	return i != data.end();
}

std::string INIParser::getString(std::string_view::iterator& i, std::string_view data, long long error_line)
{
	std::string localString;
	while (i != data.end() && (*i != ' ' && *i != '\n' && *i != '\t' && *i != '[' && *i != ']' && *i != '=' && *i != ';'))
	{
		localString += *i;
		i++;
	}
	return localString;
}

std::string qini::INIParser::get_logic_error_string(long long error_line)
{
	return std::format("Invalid Input, in line {}", error_line);
}

std::string INIWriter::write(const INIObject& ob)
{
	std::string localString;
	for (const auto& [section, keys] : ob.m_sections)
	{
		localString += std::format("[{}]\n", section);
		for (const auto& [key, value] : keys)
		{
			localString += std::format("{}={}\n", key, value);
		}
	}
	return localString;
}

bool INIWriter::write(const INIObject& ob, std::ofstream& file)
{
	if (!file)
		return false;

	file.clear();
	for (const auto& [section, keys] : ob.m_sections)
	{
		file << std::format("[{}]\n", section);
		for (const auto& [key, value] : keys)
		{
			file << std::format("{}={}\n", key, value);
		}
	}
	file << std::endl;

	return true;
}

std::string INIWriter::fastWrite(const INIObject& ob)
{
	static INIWriter writer;
	return writer.write(ob);
}

bool INIWriter::fastWrite(const INIObject& ob, std::ofstream& file)
{
	static INIWriter writer;
	return writer.write(ob, file);
}

INI_NAMESPACE_END
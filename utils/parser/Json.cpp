#include "Json.h"

#include <cmath>

#define JSON_NAMESPACE_START namespace qjson {
#define JSON_NAMESPACE_END }

JSON_NAMESPACE_START

JObject::JObject()
    :m_type(JValueType::JNull),
    m_value(std::make_unique<value_t>())
{
    *m_value = null_t();
}

JObject::JObject(const JObject& jo)
    :m_type(jo.m_type),
    m_value(std::make_unique<value_t>())
{
    switch (jo.m_type) {
    case JValueType::JString:
        *m_value = *std::get_if<string_t>(jo.m_value.get());
        break;
    case JValueType::JList:
        *m_value = *std::get_if<list_t>(jo.m_value.get());
        break;
    case JValueType::JDict:
        *m_value = *std::get_if<dict_t>(jo.m_value.get());
        break;
    default:
        *m_value = *jo.m_value;
        break;
    }
}

JObject::JObject(JObject&& jo) noexcept
    :m_type(jo.m_type)
{
    m_value = std::move(jo.m_value);
}

JObject::JObject(JValueType jvt)
    :m_type(jvt),
    m_value(std::make_unique<value_t>())
{
    switch (jvt) {
    case qjson::JValueType::JNull:
        *m_value = null_t();
        break;
    case qjson::JValueType::JInt:
        *m_value = int_t();
        break;
    case qjson::JValueType::JDouble:
        *m_value = double_t();
        break;
    case qjson::JValueType::JBool:
        *m_value = bool_t();
        break;
    case qjson::JValueType::JString:
        *m_value = string_t();
        break;
    case qjson::JValueType::JList:
        *m_value = list_t();
        break;
    case qjson::JValueType::JDict:
        *m_value = dict_t();
        break;
    default:
        break;
    }
}

JObject::JObject(long long value)
    :m_type(JValueType::JInt),
    m_value(std::make_unique<value_t>())
{
    *m_value = value;
}

JObject::JObject(long value)
    :m_type(JValueType::JInt),
    m_value(std::make_unique<value_t>())
{
    *m_value = static_cast<long long>(value);
}

JObject::JObject(int value)
    :m_type(JValueType::JInt),
    m_value(std::make_unique<value_t>())
{
    *m_value = static_cast<long long>(value);
}

JObject::JObject(short value)
    :m_type(JValueType::JInt),
    m_value(std::make_unique<value_t>())
{
    *m_value = static_cast<long long>(value);
}

JObject::JObject(bool value)
    :m_type(JValueType::JBool),
    m_value(std::make_unique<value_t>())
{
    *m_value = value;
}

JObject::JObject(long double value)
    :m_type(JValueType::JDouble),
    m_value(std::make_unique<value_t>())
{
    *m_value = value;
}

JObject::JObject(double value)
    :m_type(JValueType::JDouble),
    m_value(std::make_unique<value_t>())
{
    *m_value = static_cast<long double>(value);
}

JObject::JObject(float value)
    :m_type(JValueType::JDouble),
    m_value(std::make_unique<value_t>())
{
    *m_value = static_cast<long double>(value);
}

JObject::JObject(const char* data)
    :m_type(JValueType::JString),
    m_value(std::make_unique<value_t>())
{
    *m_value = std::string(data);
}

JObject::JObject(const std::string& data)
    :m_type(JValueType::JString),
    m_value(std::make_unique<value_t>())
{
    *m_value = data;
}

qjson::JObject::JObject(std::string_view data)
    :m_type(JValueType::JString),
    m_value(std::make_unique<value_t>())
{
    *m_value = std::string(data);
}

JObject::JObject(std::string&& data) noexcept
    :m_type(JValueType::JString),
    m_value(std::make_unique<value_t>())
{
    *m_value = std::move(data);
}

JObject::~JObject() = default;

JObject& JObject::operator=(const JObject& jo)
{
    if (this == &jo)
        return *this;

    m_type = jo.m_type;
    switch (jo.m_type) {
    case JValueType::JString:
        *m_value = *std::get_if<string_t>(jo.m_value.get());
        break;
    case JValueType::JList:
        *m_value = *std::get_if<list_t>(jo.m_value.get());
        break;
    case JValueType::JDict:
        *m_value = *std::get_if<dict_t>(jo.m_value.get());
        break;
    default:
        *m_value = *jo.m_value;
        break;
    }
    return *this;
}

JObject& JObject::operator=(JObject&& jo) noexcept
{
    if (this == &jo)
        return *this;

    m_type = jo.m_type;
    m_value = std::move(jo.m_value);
    return *this;
}

bool operator==(const JObject& joa, const JObject& jo)
{
    if (joa.m_type != jo.m_type)
        return false;
    switch (jo.m_type) {
    case JValueType::JNull:
        return true;
    case JValueType::JInt:
        if (joa.getInt() == jo.getInt())
            return true;
        return false;
    case JValueType::JDouble:
        if (joa.getDouble() == jo.getDouble())
            return true;
        return false;
    case JValueType::JBool:
        if (joa.getBool() == jo.getBool())
            return true;
        return false;
    case JValueType::JString:
        if (joa.getString() == jo.getString())
            return true;
        return false;
    case JValueType::JList: {
        const list_t& local = joa.getList();
        const list_t& jolist = jo.getList();
        if (local.empty() ^ jolist.empty())
            return false;
        if (local.size() != jolist.size())
            return false;
        for (std::size_t i = 0; i < local.size(); i++) {
            if (!(local[i] == jolist[i]))
                return false;
        }
        return true;
    }
    case JValueType::JDict: {
        const dict_t& local = joa.getDict();
        const dict_t& joDict = jo.getDict();
        if (local.empty() && joDict.empty())
            return true;
        if (local.empty() != joDict.empty())
            return false;
        if (local.size() != joDict.size())
            return false;
        for (auto i = local.begin(); i != local.end(); ++i) {
            if (joDict.find(i->first) == joDict.end())
                return false;
            else if (!(i->second == joDict.find(i->first)->second))
                return false;
        }
        return true;
    }
    default:
        return false;
    }
}

bool operator==(const JObject& jo, JValueType type)
{
    if (jo.m_type == type)
        return true;
    return false;
}

const JObject& JObject::operator[](std::size_t iter) const
{
    if (m_type != JValueType::JNull && m_type != JValueType::JList)
        throw std::logic_error("The type isn't JList.");
    if (m_type == JValueType::JNull)
        throw std::logic_error("The type is JNull.");
    list_t* local = std::get_if<list_t>(m_value.get());
    if (iter >= local->size())
        throw std::logic_error("The size is smaller than iter.");
    return (*local).at(iter);
}

JObject& JObject::operator[](std::size_t iter)
{
    if (m_type != JValueType::JNull && m_type != JValueType::JList)
        throw std::logic_error("The type isn't JList.");
    if (m_type == JValueType::JNull) {
        m_type = JValueType::JList;
        *m_value = list_t();
        std::get_if<list_t>(m_value.get())->resize(iter + 1);
        return (*std::get_if<list_t>(m_value.get())).at(iter);
    }
    list_t* local = std::get_if<list_t>(m_value.get());
    if (iter >= local->size())
        local->resize(iter + 1);
    return (*local).at(iter);
}

const JObject& JObject::operator[](int iter) const
{
    return operator[](static_cast<std::size_t>(iter));
}

JObject& JObject::operator[](int iter)
{
    return operator[](static_cast<std::size_t>(iter));
}

const JObject& JObject::operator[](const char* str) const
{
    if (m_type != JValueType::JNull && m_type != JValueType::JDict)
        throw std::logic_error("The type isn't JDict.");
    if (m_type == JValueType::JNull)
        throw std::logic_error("The type is JNull.");
    return (*std::get_if<dict_t>(m_value.get())).at(str);
}

JObject& JObject::operator[](const char* str)
{
    if (m_type != JValueType::JNull && m_type != JValueType::JDict)
        throw std::logic_error("The type isn't JDict.");
    if (m_type == JValueType::JNull) {
        m_type = JValueType::JDict;
        *m_value = dict_t();
        return (*std::get_if<dict_t>(m_value.get()))[str];
    }
    return (*std::get_if<dict_t>(m_value.get()))[str];
}

void JObject::push_back(const JObject& jo)
{
    if (m_type != JValueType::JNull && m_type != JValueType::JList)
        throw std::logic_error("The type isn't JList.");
    if (m_type == JValueType::JNull) {
        m_type = JValueType::JList;
        *m_value = list_t();
        std::get_if<list_t>(m_value.get())->push_back(jo);
    }
    std::get_if<list_t>(m_value.get())->push_back(jo);
}

void JObject::push_back(JObject&& jo)
{
    if (m_type != JValueType::JNull && m_type != JValueType::JList)
        throw std::logic_error("The type isn't JList.");
    if (m_type == JValueType::JNull) {
        m_type = JValueType::JList;
        *m_value = list_t();
        std::get_if<list_t>(m_value.get())->push_back(std::move(jo));
    }
    std::get_if<list_t>(m_value.get())->push_back(std::move(jo));
}

void JObject::pop_back()
{
    if (m_type == JValueType::JList) {
        list_t* local = std::get_if<list_t>(m_value.get());
        if (local->empty())
            throw std::logic_error("The JList is empty.");
        local->pop_back();
        return;
    }
    throw std::logic_error("The type isn't JList.");
}

bool JObject::hasMember(const std::string& str) const
{
    if (m_type != JValueType::JDict)
        throw std::logic_error("The type isn't JDict.");
    const dict_t* local = std::get_if<dict_t>(m_value.get());
    if (local->find(str) != local->cend())
        return true;
    return false;
}

JValueType JObject::getType() const noexcept
{
    return m_type;
}

const list_t& JObject::getList() const
{
    if (m_type != JValueType::JList)
        throw std::logic_error("The type isn't JList.");
    return *std::get_if<list_t>(m_value.get());
}

list_t& JObject::getList()
{
    if (m_type != JValueType::JList)
        throw std::logic_error("The type isn't JList.");
    return *std::get_if<list_t>(m_value.get());
}

const dict_t& JObject::getDict() const
{
    if (m_type != JValueType::JDict)
        throw std::logic_error("The type isn't JDict.");
    return *std::get_if<dict_t>(m_value.get());
}

dict_t& JObject::getDict()
{
    if (m_type != JValueType::JDict)
        throw std::logic_error("The type isn't JDict.");
    return *std::get_if<dict_t>(m_value.get());
}

const long long& JObject::getInt() const
{
    if (m_type != JValueType::JInt)
        throw std::logic_error("This JObject isn't int");
    return *std::get_if<int_t>(m_value.get());
}

long long& JObject::getInt()
{
    if (m_type != JValueType::JInt)
        throw std::logic_error("This JObject isn't int");
    return *std::get_if<int_t>(m_value.get());
}

const long double& JObject::getDouble() const
{
    if (m_type != JValueType::JDouble)
        throw std::logic_error("This JObject isn't double");
    return *std::get_if<double_t>(m_value.get());
}

long double& JObject::getDouble()
{
    if (m_type != JValueType::JDouble)
        throw std::logic_error("This JObject isn't double");
    return *std::get_if<double_t>(m_value.get());
}

const bool& JObject::getBool() const
{
    if (m_type != JValueType::JBool)
        throw std::logic_error("This JObject isn't bool");
    return *std::get_if<bool_t>(m_value.get());
}

bool& JObject::getBool()
{
    if (m_type != JValueType::JBool)
        throw std::logic_error("This JObject isn't bool");
    return *std::get_if<bool_t>(m_value.get());
}

const std::string& JObject::getString() const
{
    if (m_type != JValueType::JString)
        throw std::logic_error("This JObject isn't string");
    return *std::get_if<string_t>(m_value.get());
}

std::string& JObject::getString()
{
    if (m_type != JValueType::JString)
        throw std::logic_error("This JObject isn't string");
    return *std::get_if<string_t>(m_value.get());
}

JObject JParser::parse(std::string_view data)
{
    std::size_t iter = 0;
    return parse_(data, data.size(), iter);
}

JObject JParser::fastParse(std::ifstream& infile)
{
    infile.seekg(0, std::ios_base::end);
    std::size_t size = infile.tellg();
    infile.seekg(0, std::ios_base::beg);
    std::string buffer;
    buffer.resize(size);
    infile.read(buffer.data(), size);
    infile.close();

    return JParser::fastParse(buffer);
}

JObject JParser::fastParse(std::string_view data)
{
    JParser jp;
    return jp.parse(data);
}

JObject JParser::parse_(std::string_view data, std::size_t data_size, std::size_t& iter)
{
    long long error_line = 0;

    if (data.empty())
        throw std::logic_error(getLogicErrorString(error_line));
    skipSpace(data, data_size, iter, error_line);
    if (data_size <= iter)
        throw std::logic_error(getLogicErrorString(error_line));

    if (data[iter] == '{') {
        JObject localJO(JValueType::JDict);
        ++iter;
        while (iter < data_size && data[iter] != '}') {
            skipSpace(data, data_size, iter, error_line);
            if (data[iter] == '}')
                return localJO;
            std::string key(getString(data, data_size, iter, error_line));
            skipSpace(data, data_size, iter, error_line);
            if (data[iter] == ':')
                ++iter;
            else
                throw std::logic_error(getLogicErrorString(error_line));
            skipSpace(data, data_size, iter, error_line);
            localJO[key.c_str()] = parse_(data, data_size, iter);
            skipSpace(data, data_size, iter, error_line);
            if (data[iter] != ',' && data[iter] != '}')
                throw std::logic_error(getLogicErrorString(error_line));
            else if (data[iter] == '}') {
                ++iter;
                return localJO;
            }
            ++iter;
            skipSpace(data, data_size, iter, error_line);
        }
        if (data[iter] == '}') {
            ++iter;
            return localJO;
        } else
            throw std::logic_error(getLogicErrorString(error_line));
    } else if (data[iter] == '[') {
        JObject localJO(JValueType::JList);
        ++iter;
        while (iter < data_size && data[iter] != ']') {
            skipSpace(data, data_size, iter, error_line);
            if (data[iter] == ']')
                return localJO;
            localJO.push_back(parse_(data, data_size, iter));
            skipSpace(data, data_size, iter, error_line);
            if (data[iter] != ',' && data[iter] != ']')
                throw std::logic_error(getLogicErrorString(error_line));
            else if (data[iter] == ']') {
                ++iter;
                return localJO;
            }
            ++iter;
            skipSpace(data, data_size, iter, error_line);
        }
        if (data[iter] == ']')
            return localJO;
        else
            throw std::logic_error(getLogicErrorString(error_line));
    } else if (data[iter] == '\"')
        return getString(data, data_size, iter, error_line);
    else if (data[iter] == 'n')
        return getNull(data, data_size, iter, error_line);
    else if (data[iter] == 't' || data[iter] == 'f')
        return getBool(data, data_size, iter, error_line);
    else if ((data[iter] >= '0' && data[iter] <= '9') || data[iter] == '-')
        return getNumber(data, data_size, iter, error_line);
    else
        throw std::logic_error(getLogicErrorString(error_line));
}

void JParser::skipSpace(std::string_view data, std::size_t data_size, std::size_t& iter, long long& error_line)
{
    while (iter < data_size && (data[iter] == ' ' || data[iter] == '\t' || data[iter] == '\n')) {
        if (data[iter] == '\n')
            ++error_line;
        ++iter;
    }
}

std::string JParser::getString(std::string_view data, std::size_t data_size, std::size_t& iter, long long error_line)
{
    if (data[iter] == '\"') {
        std::string str;
        ++iter;
        while (iter < data_size && data[iter] != '\"') {
            if (data[iter] == '\\') {
                ++iter;
                switch (data[iter]) {
                case 'n':
                    str += '\n';
                    break;
                case 'b':
                    str += "\b";
                    break;
                case 'f':
                    str += "\f";
                    break;
                case 'r':
                    str += "\r";
                    break;
                case 't':
                    str += "\t";
                    break;
                case '\\':
                    str += "\\";
                    break;
                case '\"':
                    str += "\"";
                    break;
                case '/':
                    str += "/";
                    break;
                default:
                    throw std::logic_error(getLogicErrorString(error_line));
                    break;
                }
            } else {
                str += data[iter];
            }
            ++iter;
        }
        if (iter >= data_size)
            throw std::logic_error(getLogicErrorString(error_line));
        ++iter;
        return str;
    }
    else
        throw std::logic_error(getLogicErrorString(error_line));
}

JObject JParser::getNumber(std::string_view data, std::size_t data_size, std::size_t& iter, long long error_line)
{
    bool isDouble = false;
    bool firstNum = false;
    bool isNegative = false;
    if (data[iter] == '-') {
        isNegative = true;
        ++iter;
    }
    std::size_t count = 0;
    std::size_t start = iter;

    while (iter < data_size &&
           ((data[iter] >= '0' && data[iter] <= '9') || data[iter] == '.')) {
        if (!firstNum && data[iter] >= '0' && data[iter] <= '9') {
            firstNum = true;
        }
        else if (isDouble) {
            count++;
        }
        else if (data[iter] == '.') {
            if (!firstNum)
                throw std::logic_error(getLogicErrorString(error_line));
            isDouble = true;
            ++iter;
            continue;
        }
        ++iter;
    }

    if (isDouble) {
        long double number = data[iter - 1] - '0';
        std::size_t single = 10;
        for (long long i = iter - 2; i >= static_cast<long long>(start); --i, single *= 10) {
            if (data[i] == '.')
                continue;
            number += single * (data[i] - '0');
        }
        if (isNegative)
            number *= -1;
        return number / std::pow(10, count);
    } else {
        long long number = data[iter - 1] - '0';
        std::size_t single = 10;
        for (long long i = iter - 2; i >= static_cast<long long>(start); --i, single *= 10) {
            number += single * (data[i] - '0');
        }
        if (isNegative)
            number *= -1;
        return number;
    }
}

JObject JParser::getBool(std::string_view data, std::size_t data_size, std::size_t& iter, long long error_line)
{
    if (data_size >= iter + 4 &&
        std::memcmp(data.data(), "true", 4ull)) {
        iter += 4;
        return true;
    } else if (data_size >= iter + 5 &&
               std::memcmp(data.data(), "false", 5ull)) {
        iter += 5;
        return false;
    }
    throw std::logic_error(getLogicErrorString(error_line));
}

JObject JParser::getNull(std::string_view data, std::size_t data_size, std::size_t& iter, long long error_line)
{
    if (data_size >= iter + 4 &&
        std::memcmp(data.data(), "null", 4ull)) {
            iter += 4;
            return JObject();
    }
    throw std::logic_error(getLogicErrorString(error_line));
}

std::string JParser::getLogicErrorString(long long error_line)
{
    return "Invalid Input, in line " + std::to_string(error_line);
}

std::string JWriter::write(const JObject& jo)
{
    std::string str;
    switch (jo.getType()) {
    case JValueType::JNull:
        str += "null";
        break;
    case JValueType::JInt:
        str += std::to_string(jo.getInt());
        break;
    case JValueType::JDouble:
        str += std::to_string(jo.getDouble());
        break;
    case JValueType::JBool:
        if (jo.getBool())
            str += "true";
        else
            str += "false";
        break;
    case JValueType::JString: {
        std::string localString(jo.getString());
        if (localString.empty()) {
            str += "\"\"";
        } else {
            str += '\"';
            for (const char& i: localString) {
                switch (i) {
                case 0:
                    throw std::logic_error("Invalid string");
                case '\n':
                    str += "\\n";
                    break;
                case '\b':
                    str += "\\b";
                    break;
                case '\f':
                    str += "\\f";
                    break;
                case '\r':
                    str += "\\r";
                    break;
                case '\t':
                    str += "\\t";
                    break;
                case '\\':
                    str += "\\\\";
                    break;
                case '\"':
                    str += "\\\"";
                    break;
                default:
                    str += i;
                    break;
                }
            }
            str += '\"';
        }
        break;
    }
    case JValueType::JList: {
        const list_t& list = jo.getList();
        if (list.empty()) {
            str += "[]";
        } else {
            str += '[';
            for (auto iter = list.begin(); iter != list.end(); ++iter) {
                str += write(*iter);
                if (iter + 1 != list.end()) {
                    str += ',';
                }
            }
            str += ']';
        }
        break;
    }
    case JValueType::JDict: {
        const dict_t& dict = jo.getDict();
        if (dict.empty()) {
            str += "{}";
        } else {
            str += '{';
            for (auto iter = dict.begin(), iter2 = dict.begin(); iter != dict.end(); ++iter) {
                str += '\"' + iter->first + "\":" + write(iter->second);
                iter2 = iter;
                if (++iter2 != dict.end())
                    str += ',';
            }
            str += '}';
        }
        break;
    }
    default:
        break;
    }

    return str;
}

std::string JWriter::formatWrite(const JObject& jo, size_t indent, std::size_t n)
{
    std::string str;
    std::string indent_space;
    indent_space.resize(indent);
    std::memset(indent_space.data(), ' ', indent);

    switch (jo.getType()) {
    case JValueType::JNull:
        str += "null";
        break;
    case JValueType::JInt:
        str += std::to_string(jo.getInt());
        break;
    case JValueType::JDouble:
        str += std::to_string(jo.getDouble());
        break;
    case JValueType::JBool:
        if (jo.getBool()) {
            str += "true";
            break;
        }
        str += "false";
        break;
    case JValueType::JString: {
        std::string localString(jo.getString());
        str += '\"';
        for (const char& i: localString) {
            switch (i) {
            case 0:
                throw std::logic_error("Invalid string");
            case '\n':
                str += "\\n";
                break;
            case '\b':
                str += "\\b";
                break;
            case '\f':
                str += "\\f";
                break;
            case '\r':
                str += "\\r";
                break;
            case '\t':
                str += "\\t";
                break;
            case '\\':
                str += "\\\\";
                break;
            case '\"':
                str += "\\\"";
                break;
            default:
                str += i;
                break;
            }
        }
        str += '\"';
        break;
    }
    case JValueType::JList: {
        const list_t& list = jo.getList();
        str += "[\n";
        for (auto iter = list.begin(); iter != list.end(); ++iter) {
            for (std::size_t i = 0; i < n; i++) {
                str += indent_space;
            }
            str += formatWrite(*iter, indent, n + 1);
            if (iter + 1 != list.end()) {
                str += ",\n";
            }
        }
        str += '\n';
        for (std::size_t i = 0; i < n - 1; i++) {
            str += indent_space;
        }
        str += ']';
        break;
    }
    case JValueType::JDict: {
        const dict_t& dict = jo.getDict();
        str += "{\n";
        for (auto iter = dict.begin(), iter2 = dict.begin(); iter != dict.end(); ++iter) {
            for (std::size_t i = 0; i < n; i++) {
                str += indent_space;
            }
            str += '\"' + iter->first + "\": " + formatWrite(iter->second, indent, n + 1);
            iter2 = iter;
            if (++iter2 != dict.end()) {
                str += ",\n";
            }
        }
        str += '\n';
        for (std::size_t i = 0; i < n - 1; i++) {
            str += indent_space;
        }
        str += '}';
        break;
    }
    default:
        break;
    }

    return str;
}

std::string qjson::JWriter::fastWrite(const JObject &jo)
{
    JWriter jw;
    return jw.write(jo) + '\n';
}

std::string qjson::JWriter::fastFormatWrite(const JObject &jo, std::size_t indent)
{
    JWriter jw;
    return jw.formatWrite(jo, indent) + '\n';
}

JSON_NAMESPACE_END

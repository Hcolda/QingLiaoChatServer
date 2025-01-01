#ifndef INI_HPP
#define INI_HPP

#include <unordered_map>
#include <string>
#include <string_view>
#include <fstream>
#include <stdexcept>

namespace qini
{
    class INIParser;
    class INIWriter;

    /**
     * @brief Class representing an INI object.
     */
    class INIObject
    {
    public:
        /**
         * @brief Class representing a mutable section within an INI object.
         */
        class Section
        {
        public:
            /**
             * @brief Iterator class for iterating over keys in a mutable section.
             */
            class iterator
            {
            public:
                iterator(const std::unordered_map<std::string, std::string>::iterator& itor);

                iterator(std::unordered_map<std::string, std::string>::iterator&& itor);

                iterator& operator++();

                std::string operator*();

                friend bool operator==(const iterator& a, const iterator& b);

                friend bool operator!=(const iterator& a, const iterator& b);

            private:
                std::unordered_map<std::string, std::string>::iterator m_itor;
            };

            Section(std::unordered_map<std::string, std::string>& section);

            Section(const Section&) = delete;
            Section(Section&&) = delete;

            Section& operator=(const Section&) = delete;
            Section& operator=(Section&&) = delete;

            std::string& operator[](const std::string& keyName);

            const std::string& operator[](const std::string& keyName) const;

            iterator begin();

            iterator end();

        private:
            std::unordered_map<std::string, std::string>& m_keys;
        };

        /**
         * @brief Class representing an immutable section within an INI object.
         */
        class ConstSection
        {
        public:
            /**
             * @brief Iterator class for iterating over keys in an immutable section.
             */
            class const_iterator
            {
            public:
                const_iterator(const std::unordered_map<std::string, std::string>::const_iterator& itor);

                const_iterator(std::unordered_map<std::string, std::string>::const_iterator&& itor);

                const_iterator& operator++();

                std::string operator*();

                friend bool operator==(const const_iterator& a, const const_iterator& b);

                friend bool operator!=(const const_iterator& a, const const_iterator& b);

            private:
                std::unordered_map<std::string, std::string>::const_iterator m_itor;
            };

            ConstSection(const std::unordered_map<std::string, std::string>& section);

            ConstSection(const Section&) = delete;
            ConstSection(Section&&) = delete;

            ConstSection& operator=(const Section&) = delete;
            ConstSection& operator=(Section&&) = delete;

            const std::string& operator[](const std::string& keyName) const;

            const_iterator begin();

            const_iterator end();

        private:
            const std::unordered_map<std::string, std::string>& m_keys;
        };

        /**
         * @brief Iterator class for iterating over sections in an INI object.
         */
        class iterator
        {
        public:
            iterator(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator& itor);

            iterator(std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator&& itor);

            iterator& operator++();

            Section operator*();

            friend bool operator==(const iterator& a, const iterator& b);

            friend bool operator!=(const iterator& a, const iterator& b);

        private:
            std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::iterator m_itor;
        };

        /**
         * @brief Iterator class for iterating over const sections in an INI object.
         */
        class const_iterator
        {
        public:
            const_iterator(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::const_iterator& itor);

            const_iterator(std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::const_iterator&& itor);

            const_iterator& operator++();

            ConstSection operator*();

            friend bool operator==(const const_iterator& a, const const_iterator& b);

            friend bool operator!=(const const_iterator& a, const const_iterator& b);

        private:
            std::unordered_map<std::string, std::unordered_map<std::string, std::string>>::const_iterator m_itor;
        };

        INIObject() = default;

        INIObject(const INIObject& ob);

        INIObject(INIObject&& ob) noexcept;

        ~INIObject() = default;

        INIObject& operator=(const INIObject& ob);

        INIObject& operator=(INIObject&& ob) noexcept;

        Section operator[](const std::string& sectionName);

        ConstSection operator[](const std::string& sectionName) const;

        iterator begin();

        iterator end();

        friend bool operator==(const INIObject& ia, const INIObject& ib);

        friend bool operator!=(const INIObject& ia, const INIObject& ib);

    private:
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_sections;

        friend class INIParser;
        friend class INIWriter;
    };

    /**
     * @brief Class for parsing INI data.
     */
    class INIParser
    {
    public:
        INIParser() = default;
        ~INIParser() = default;

        /**
         * @brief Parses INI data from a string view.
         * @param data The INI data to parse.
         * @return The parsed INI object.
         */
        INIObject parse(std::string_view data);

        /**
         * @brief Quickly parses INI data from a string view.
         * @param data The INI data to parse.
         * @return The parsed INI object.
         */
        static INIObject fastParse(std::string_view data);

        /**
         * @brief Quickly parses INI data from an input file stream.
         * @param infile The input file stream.
         * @return The parsed INI object.
         */
        static INIObject fastParse(std::ifstream& infile);

    protected:
        bool skipSpace(std::string_view::iterator& i, std::string_view data, long long& error_line);

        std::string getString(std::string_view::iterator& i, std::string_view data, long long error_line);

        std::string getLogicErrorString(long long error_line);
    };

    /**
     * @brief Class for writing INI data.
     */
    class INIWriter
    {
    public:
        INIWriter() = default;
        ~INIWriter() = default;

        /**
         * @brief Writes INI data from an INI object to a string.
         * @param ob The INI object to write.
         * @return The INI data as a string.
         */
        std::string write(const INIObject& ob);

        /**
         * @brief Writes INI data from an INI object to a file.
         * @param ob The INI object to write.
         * @param file The output file stream.
         * @return true if successful, false otherwise.
         */
        bool write(const INIObject& ob, std::ofstream& file);

        /**
         * @brief Quickly writes INI data from an INI object to a string.
         * @param ob The INI object to write.
         * @return The INI data as a string.
         */
        static std::string fastWrite(const INIObject& ob);

        /**
         * @brief Quickly writes INI data from an INI object to a file.
         * @param ob The INI object to write.
         * @param file The output file stream.
         * @return true if successful, false otherwise.
         */
        static bool fastWrite(const INIObject& ob, std::ofstream& file);
    };
}

#endif // !INI_HPP

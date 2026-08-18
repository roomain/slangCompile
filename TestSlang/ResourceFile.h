#pragma once

#include <concepts>
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>

template<typename T>
concept binary_constructible = requires(const std::vector<char>&a_data) {
    T(a_data);
};

constexpr size_t NAME_MAX_SIZE = 64;

struct Heading
{
    char filename[NAME_MAX_SIZE];
    uint32_t crc;
    uint64_t offset;
};

using Binary = std::vector<int8_t>;

/*File resouces*/
/* goal is to find and check file modification fast*/
/*
------------------------------------------
| GPU info                               |
------------------------------------------
------------------------------------------
| filename                               |
| CRC crc of file to detect modification |
| offset to block                        |
------------------------------------------
------------------------------------------
| filename                               |
| CRC crc of file to detect modification |
| offset to block                        |
------------------------------------------
...
------------------------------------------
| Block:                                 |
| Hash corresponding to filename         |
| binary                                 |
------------------------------------------
------------------------------------------
| Block:                                 |
| Hash corresponding to filename         |
| binary                                 |
------------------------------------------
...
*/


//template<typename Resource> requires binary_constructible<Resource>::value
class ResourceFile
{
private:
    std::ifstream m_loader;
    std::string m_filename;
    std::vector<Heading> m_headings;                        /*!< file headers*/
    std::unordered_map<std::string, Binary> m_binaryMap;    /*!< binaries by header names*/

    void closeFile();
    void write(std::ofstream& a_output)const;

public:
    ResourceFile() = default;
    ~ResourceFile();
    const std::string& filename()const;

    bool loadAllFile(const std::string& a_filename);
    bool loadHeaders(const std::string& a_filename);
    bool loadBinaries();

    bool save()const;
    bool saveAs(const std::string& a_filename)const;
    constexpr size_t headerCount()const { return m_headings.size(); }
    constexpr size_t binayCount()const { return m_binaryMap.size(); }


    using const_binaryIterator = std::unordered_map<std::string, Binary>::const_iterator;
    const_binaryIterator findBinary(const std::string& a_binaryName);
    const Binary& binaryAt(const std::string& a_binaryName);

    void clear();


    using iterator = std::vector<Heading>::iterator;
    using const_iterator = std::vector<Heading>::const_iterator;
    iterator begin() { return m_headings.begin(); }
    iterator end() { return m_headings.end(); }
    const_iterator cbegin()const { return m_headings.cbegin(); }
    const_iterator cend()const { return m_headings.cend(); }

    void emplace(const std::string& a_filename, const uint32_t a_crc, const Binary& a_binary);
};


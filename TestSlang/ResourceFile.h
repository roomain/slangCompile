#pragma once

#include <concepts>
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include "Resources.h"
#include <fstream>

template<typename T>
concept binary_constructible = requires(const std::vector<char>&a_data) {
    T(a_data);
};


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

class ResourceFile : public Resources
{
private:
    std::ifstream m_loader;
    size_t m_inputFileSize{ 0 };
    std::string m_filename;

protected:

    void closeFile();
    virtual void write(std::ofstream& a_output)const;
    void writeHeaders(std::ofstream& a_output)const;
    void writeBinaries(std::ofstream& a_output)const;
    bool loadHeaders();

public:
    ResourceFile() = default;
    virtual ~ResourceFile();
    const std::string& filename()const;

    virtual bool loadAllFile(const std::string& a_filename);
    virtual bool loadHeaders(const std::string& a_filename);
    virtual bool loadBinaries();

    bool save()const;
    bool saveAs(const std::string& a_filename)const;

    void clear()override;


    void emplace(const std::string& a_filename, const uint32_t a_crc, const Binary& a_binary);
    void emplace(const std::string& a_filename, const uint32_t a_crc, Binary&& a_binary);
};


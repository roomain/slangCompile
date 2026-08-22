#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <string>

constexpr size_t NAME_MAX_SIZE = 64;

/*@brief header for binary description*/
struct BinHeader
{
    std::array<char, NAME_MAX_SIZE> filename;   /*!< file name*/
    uint32_t crc;                               /*!< crc to detect file modifications*/
    
    BinHeader();
    BinHeader(const std::string& a_filename, const uint32_t a_crc);
};

using HeaderList = std::vector<BinHeader>;
using Binary = std::vector<char>;


enum class DeltaType
{
    delta_removed,
    delta_updated,
    delta_new
};


/*@brief base class for resource sara*/
class Resources
{
protected:
    HeaderList m_headings;                                  /*!< file headers*/
    std::unordered_map<std::string, Binary> m_binaryMap;    /*!< binaries by header names*/

    Resources() = default;
    Resources(const Resources&) = default;
    Resources(Resources&&)noexcept = default;
    Resources& operator = (const Resources&) = default;
    Resources& operator = (Resources&&)noexcept = default;

    void emplace(const BinHeader& a_header, const Binary& a_binary);
    void emplace(BinHeader&& a_header, const Binary& a_binary);
    void emplace(BinHeader&& a_header, const Binary&& a_binary);

public:   
    virtual ~Resources() = default;

    struct Delta
    {
        BinHeader heading;
        DeltaType type;
    };

    std::vector<Delta> diff(const HeaderList& a_header);
    void merge(const Resources& a_other);

    constexpr size_t headerCount()const { return m_headings.size(); }
    constexpr size_t binayCount()const { return m_binaryMap.size(); }

    using const_binaryIterator = std::unordered_map<std::string, Binary>::const_iterator;
    const_binaryIterator findBinary(const std::string& a_binaryName)const;
    const_binaryIterator binaryEnd()const;
    const Binary& binaryAt(const std::string& a_binaryName);

    virtual void clear();

    using iterator = HeaderList::iterator;
    using const_iterator = HeaderList::const_iterator;
    iterator begin() { return m_headings.begin(); }
    iterator end() { return m_headings.end(); }
    const_iterator cbegin()const { return m_headings.cbegin(); }
    const_iterator cend()const { return m_headings.cend(); }
};
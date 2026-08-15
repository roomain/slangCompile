#pragma once
#include <string>
#include <memory>
#include <vector>
#include <libbson-1.0/bson.h>



struct BsonDeleter
{
	void operator()(bson_t* ptr) const noexcept
	{
		if (ptr)
			bson_destroy(ptr);
	}
};

class BsonDocument
{
public:
	BsonDocument() = delete;
	explicit BsonDocument(const std::string& a_filename);
	~BsonDocument() = default;
	void load(const std::string& a_filename);
	void add(const std::string& a_key, const uint32_t a_crc, const std::vector<uint8_t>& a_data);
	bool read(const std::string& a_key, uint32_t& a_crc, std::vector<uint8_t>& a_data);
	void remove(const std::string& a_key);
	bool save();
	const std::string& filename()const { return m_filename; }

private:
	std::string m_filename;
	static inline const std::string CRC_ATTRIB{ "crc" };
	static inline const std::string BIN_ATTRIB{ "binary" };

	using BsonPtr = std::unique_ptr<bson_t, BsonDeleter>;
	BsonPtr m_pBson;
};
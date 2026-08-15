#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <concepts>
#include <functional>

template<typename T>
concept binary_constructible = requires(const std::vector<char>& a_data) {
	T(a_data);
};

template<typename T>
concept binary_serialisable = requires(std::vector<char>&a_data) {
	T::serialize(a_data);
};

using loadBinary = std::function<bool(const std::string&, const uint32_t, const std::vector<char>&)>;

/*@brief load a binary json, load json resource node blob data via a loader */
void loadBinaryJson(const std::string& a_filename, const loadBinary& a_loader);

class IDynamicResource
{
protected:
	std::string m_filename; /*!< resource bjson filename*/

public:
	IDynamicResource() = default;
	explicit IDynamicResource(const std::string& a_filaname);
	void load(const std::string& a_filename);
};


/*@brief load and store resources from binary json*/
template<binary_constructible T>
class DynamicResources : public IDynamicResource
{
public:
	struct Resource
	{
		uint32_t crc = 0;
		T data;
	};

private:
	std::unordered_map<std::string, Resource> m_resourceDict;	/*!< resource dictionary*/

	bool loadResource(const std::string& a_key, const uint32_t a_crc, const std::vector<char>& a_data)
	{
		return m_resourceDict.emplace(a_key, Resource{ a_crc, a_data }).second;
	}

public:
	DynamicResources() = default;
	explicit DynamicResources(const std::string& a_filaname)
	{
		using namespace std::placeholders;
		loadBinaryJson(a_filaname, std::find(DynamicResources<T>::loadResource, this, _1, _2, _3));
	}

	void load(const std::string& a_filename)
	{
		using namespace std::placeholders;
		loadBinaryJson(a_filaname, std::find(DynamicResources<T>::loadResource, this, _1, _2, _3));
	}

	const Resource& operator [] (const std::string& a_key)const
	{
		return m_resourceDict.at(a_key);
	}

	using iterator = std::unordered_map<std::string, Resource>::iterator;
	using const_iterator = std::unordered_map<std::string, Resource>::const_iterator;
	iterator begin() { return m_resourceDict.begin(); }
	iterator end() { return m_resourceDict.end(); }
	const_iterator cbegin()const { return m_resourceDict.cbegin(); }
	const_iterator cend()const { return m_resourceDict.cend(); }
};
#pragma once

class JsonArray;
class JsonObject;

template<class T>
concept NumberType = std::is_arithmetic<T>::value && !std::is_same<T, bool>::value;
template<class T>
concept JsonSimpleType = std::is_arithmetic<T>::value || std::is_convertible<T, std::string>::value;
template<class T>
concept JsonDataType = std::is_arithmetic<T>::value || std::is_convertible<T, std::string>::value
						|| std::is_same<T, JsonObject>::value || std::is_same<T, JsonArray>::value;

class JsonException: public std::exception
{
public:
	JsonException(std::string msg) : std::exception(msg.c_str()) {}
	static JsonException ExpectedButFound(char exp, char found)
	{
		return JsonException("Expected character(" + std::string(1, exp) + ") but found (" + std::string(1, found) + ")");
	}
};

class JsonParser
{
public:
	template<NumberType T>
	static std::string Parse(const T& value)
	{
		return std::to_string(value);
	}
	template<std::convertible_to<std::string> T>
	static std::string Parse(const T& value)
	{
		if constexpr (std::is_same<T, std::string>::value)
			return "\"" + value + "\"";
		return "\"" + std::string(value) + "\"";
	}
	static std::string Parse(bool value)
	{
		return value ? "true" : "false";
	}
	static std::string Parse(JsonArray value);
	static std::string Parse(JsonObject value);

	template<JsonSimpleType T>
	static T Get(const std::string& str)
	{
		if constexpr (std::is_same<T, bool>::value)
		{
			if (str == "true")
				return true;
			else if (str == "false")
				return false;
		}
		else if constexpr (std::is_floating_point<T>::value)
			return std::stod(str);
		else if constexpr (std::is_unsigned<T>::value)
			return std::stoull(str);
		else if constexpr (std::is_signed<T>::value)
			return std::stoll(str);
		else if constexpr (std::is_convertible<T, std::string>::value)
		{
			return str.substr(1, str.size() - 2);
		}
		else
			_ASSERT(false);
	}
	static JsonObject GetObject(const std::string& str);
	static JsonArray GetArray(const std::string& str);
};

class JsonObject
{
public:
	template<JsonDataType T>
	JsonObject& Property(const char* name, const T& value)
	{
		properties[name] = JsonParser::Parse(value);
		order.push_back(name);
		return *this;
	}
	template<JsonDataType T>
	T Get(const std::string& name) const
	{
		if (!properties.contains(name))
			throw JsonException("Object does not contain property \"" + name + "\"");
		try
		{
			if constexpr (std::is_same<T, JsonObject>::value)
				return JsonParser::GetObject(properties.at(name));
			else if constexpr (std::is_same<T, JsonArray>::value)
				return JsonParser::GetArray(properties.at(name));
			else
				return JsonParser::Get<T>(properties.at(name));
		} catch (std::exception e)
		{
			throw JsonException("Failed to parse \"" + name + "\":" + properties.at(name) + " to " + typeid(T).name() + ". (" + e.what() + ")");
		}
	}
	std::string ToString() const;
	static JsonObject Parse(std::string json);
private:
	std::unordered_map<std::string, std::string> properties;
	std::vector<std::string> order;
};

class JsonArray
{
public:
	template<JsonDataType T>
	JsonArray& Element(const T& value)
	{
		elements.push_back(JsonParser::Parse(value));
		return *this;
	}
	template<JsonDataType T>
	JsonArray& Elements(const std::span<T> array)
	{
		for (size_t i = 0; i < array.size(); i++)
		{
			elements.push_back(JsonParser::Parse(array[i]));
		}
		return *this;
	}
	template<JsonDataType T>
	JsonArray& Elements(T* data, int count)
	{
		for (size_t i = 0; i < count; i++)
		{
			elements.push_back(JsonParser::Parse(data[i]));
		}
		return *this;
	}
	template<JsonDataType T>
	JsonArray& Elements(const std::initializer_list<T> array)
	{
		for (auto& element : array)
		{
			elements.push_back(JsonParser::Parse(element));
		}
		return *this;
	}
	template<JsonDataType T>
	std::vector<T> Get() const
	{
		std::vector<T> array;
		for (size_t i = 0; i < elements.size(); i++)
		{
			try
			{
				if constexpr (std::is_same<T, JsonObject>::value)
				{
					array.push_back(JsonParser::GetObject(elements[i]));
				}
				else if constexpr (std::is_same<T, JsonArray>::value)
				{
					array.push_back(JsonParser::GetArray(elements[i]));
				}
				else
				{
					array.push_back(JsonParser::Get<T>(elements[i]));
				}
			} catch (std::exception e)
			{
				throw JsonException("Failed to parse " + elements[i] + " to " + typeid(T).name() + ". (" + e.what() + ")");
			}
		}
		return array;
	}
	template<JsonDataType T>
	void Get(T* buf, size_t size) const
	{
		for (size_t i = 0; i < elements.size(); i++)
		{
			if (i == size)
				throw JsonException("Buffer size too small");
			try
			{
				if constexpr (std::is_same<T, JsonObject>::value)
				{
					buf[i] = JsonParser::GetObject(elements[i]);
				}
				else if constexpr (std::is_same<T, JsonArray>::value)
				{
					buf[i] = JsonParser::GetArray(elements[i]);
				}
				else
				{
					buf[i] = JsonParser::Get<T>(elements[i]);
				}
			} catch (std::exception e)
			{
				throw JsonException("Failed to parse " + elements[i] + " to " + typeid(T).name() + ". (" + e.what() + ")");
			}
		}
	}
	template<JsonDataType T>
	T Get(int i) const
	{
		try
		{
			if constexpr (std::is_same<T, JsonObject>::value)
			{
				return JsonParser::GetObject(elements[i]);
			}
			else if constexpr (std::is_same<T, JsonArray>::value)
			{
				return JsonParser::GetArray(elements[i]);
			}
			else
			{
				return JsonParser::Get<T>(elements[i]);
			}
		} catch (std::exception e)
		{
			throw JsonException("Failed to parse " + elements[i] + " to " + typeid(T).name() + ". (" + e.what() + ")");
		}
	}
	std::string ToString() const;
	size_t Size() const
	{
		return elements.size();
	}
	static JsonArray Parse(std::string json);
private:
	std::vector<std::string> elements;
};
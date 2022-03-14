#include "Json.h"

std::string JsonParser::Parse(JsonArray value)
{
	return value.ToString();
}
std::string JsonParser::Parse(JsonObject value)
{
	return value.ToString();
}
JsonObject JsonParser::GetObject(const std::string& str)
{
	return JsonObject::Parse(str);
}
JsonArray JsonParser::GetArray(const std::string& str)
{
	return JsonArray::Parse(str);
}

std::string JsonObject::ToString() const
{
	std::stringstream ss;

	ss << "{";
	for (size_t i = 0; i < order.size(); i++)
	{
		ss << "\"" << order[i] << "\"" << ":" << properties.at(order[i]) << ",";
	}
	ss.seekp(-1, ss.cur) << "}";
	return ss.str();
}

std::string JsonArray::ToString() const
{
	if (elements.empty())
		return "[]";
	std::stringstream ss;

	ss << "[";
	for (size_t i = 0; i < elements.size(); i++)
	{
		ss << elements[i] << ",";
	}
	ss.seekp(-1, ss.cur) << "]";
	return ss.str();
}

static std::string GetNextPropertyName(const std::string& json, size_t& pos)
{
	if (json[pos] != '\"') 
		throw JsonException::ExpectedButFound('\"', json[pos]);
	size_t end = pos;
	do
	{
		end = json.find_first_of('\"', end + 1);
	} while (json[end - 1] == '\\');
	std::string name = json.substr(pos + 1, end - (pos + 1));
	pos = end + 1;
	return name;
}
static bool IsInsideString(const std::string& str, size_t pos, size_t from)
{
	bool isStr = true;
	size_t curr = from;
	while (curr <= pos)
	{
		curr = str.find_first_of('\"', curr + 1);
		if (curr == str.npos || str[curr - 1] != '\\')
		{
			isStr = !isStr;
		}
	}
	return isStr;
}
static size_t FindClosing(char open, char close, const std::string& str, size_t from)
{
	size_t closePos = from;
	size_t prevClosePos = closePos;
	int openArraysBetween = 1;

	do 
	{
		prevClosePos = closePos;
		do
		{
			closePos = str.find_first_of(close, closePos + 1);
		} while (IsInsideString(str, closePos, from));

		openArraysBetween--;
		size_t openPos = prevClosePos;
		while (openPos < closePos)
		{
			openPos = str.find_first_of(open, openPos + 1);
			if (openPos < closePos && !IsInsideString(str, openPos, from))
				openArraysBetween++;
		}
	} while (openArraysBetween > 0);
	return closePos;
}
static std::string GetNextPropertyValue(const std::string& json, size_t& pos)
{
	size_t end;
	std::string value;
	if (json[pos] == '\"')
	{
		end = json.find_first_of('\"', pos + 1); // not handling \" being in it
		value = json.substr(pos, ++end - pos);
	}
	else if (std::string("-0123456789").find(json[pos]) != json.npos)
	{
		end = json.find_first_not_of("0123456789.", pos + 1);
		value = json.substr(pos, end - pos);
	}
	else if (json[pos] == 't')
	{
		value = json.substr(pos, 4);
		if (!value._Equal("true")) 
			throw JsonException("expected true, but got " + value);
		end = pos + 4;
	}
	else if (json[pos] == 'f')
	{
		value = json.substr(pos, 5);
		if (!value._Equal("false")) 
			throw JsonException("expected false, but got " + value);
		end = pos + 5;
	}
	else if (json[pos] == '{')
	{
		end = FindClosing('{', '}', json, pos);
		// end = pos;
		// do
		// {
		// 	end = json.find_first_of('}', end + 1);
		// } while (IsInsideString(json, end, pos));
		value = json.substr(pos, ++end - pos);
	}
	else if (json[pos] == '[')
	{
		end = FindClosing('[', ']', json, pos);
		// end = pos;
		// do
		// {
		// 	end = json.find_first_of(']', end + 1);
		// } while (IsInsideString(json, end, pos));
		value = json.substr(pos, ++end - pos);
	}
	pos = end;
	return value;
}
JsonObject JsonObject::Parse(std::string json)
{
	JsonObject jsonObject;

	size_t pos = 0;
	if (json[pos] != '{') JsonException::ExpectedButFound('{', json[pos]);
	if (json[++pos] == '}') return jsonObject;

	do
	{
		std::string name = GetNextPropertyName(json, pos);
		if (json[pos] != ':') 
			throw JsonException::ExpectedButFound(':', json[pos]);
		std::string value = GetNextPropertyValue(json, ++pos);

		jsonObject.properties[name] = value;
		jsonObject.order.push_back(name);
	} while (json[pos] == ',' && ++pos);

	if (json[pos] != '}') 
		throw JsonException::ExpectedButFound('}', json[pos]);
	return jsonObject;
}

JsonArray JsonArray::Parse(std::string json)
{
	JsonArray jsonArray;

	size_t pos = 0;
	if (json[pos] != '[') JsonException::ExpectedButFound('[', json[pos]);
	if (json[++pos] == ']') return jsonArray;

	do
	{
		std::string value = GetNextPropertyValue(json, pos);

		jsonArray.elements.push_back(value);
	} while (json[pos] == ',' && ++pos);

	if (json[pos] != ']')
		throw JsonException::ExpectedButFound(']', json[pos]);
	return jsonArray;
}
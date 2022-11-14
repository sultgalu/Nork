#include "Json.h"

static std::string FormatJsonString(const std::string& json)
{
	std::string formatted;
	formatted.reserve((size_t)(json.size() * 1.2f));
	int depth = 0;
	auto newLine = [&]()
	{
		formatted += "\n";
		for (size_t i = 0; i < depth; i++)
		{
			formatted += "  ";
		}
	};

	bool insideString = false;
	for (size_t i = 0; i < json.size(); i++)
	{
		char c = json[i];
		if (insideString)
		{
			formatted += c; // append every character as is
			if (json[i] == '\"' && json[i - 1] != '\\')
				insideString = false;
			continue;
		}
		switch (c)
		{
		case ',':
			formatted += ',';
			newLine();
			break;
		case ':':
			formatted += ": ";
			break;
		case '{':
		case '[':
			formatted += c;
			depth++;
			newLine();
			break;
		case '}':
		case ']':
			depth--;
			newLine();
			formatted += c;
			break;
		case '\"': // skip strings
			insideString = true;
			formatted += '\"';
			break;
		default:
			formatted += c;
		}
	}
	return formatted;
}
static std::string UnformatJsonString(const std::string& formatted)
{
	std::string json;
	json.reserve(formatted.size());

	bool insideString = false;
	for (size_t i = 0; i < formatted.size(); i++)
	{
		char c = formatted[i];
		if (insideString)
		{
			json += c; // append every character as is
			if (formatted[i] == '\"' && formatted[i - 1] != '\\')
				insideString = false;
			continue;
		}
		switch (c)
		{
		case '\n':
		case '\r':
		case '\t':
		case ' ':
			break;
		case '\"': // skip strings
			insideString = true;
			json += '\"';
			break;
		default:
			json += c;
		}
	}
	return json;
}

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

std::string JsonParser::FormatString(const std::string& json)
{
	std::string formatted;
	formatted.reserve(json.size());

	for (auto c : json)
	{
		switch (c)
		{
		case '\b':
			formatted += "\\b";
			break;
		case '\f':
			formatted += "\\f";
			break;
		case '\n':
			formatted += "\\n";
			break;
		case '\t':
			formatted += "\\t";
			break;
		case '\r':
			formatted += "\\r";
			break;
		case '\"':
			formatted += "\\\"";
			break;
		case '\\':
			formatted += "\\\\";
			break;
		default: [[likely]]
			formatted += c;
			break;
		}
	}
	return formatted;
}
std::string JsonParser::UnformatString(const std::string& formatted)
{
	std::string json;
	json.reserve(formatted.size());

	for (size_t i = 0; i < formatted.size(); i++)
	{
		char c = formatted[i];
		if (c == '\\')
		{
			switch (formatted[i + 1])
			{
			case 'b':
				json += '\b';
				break;
			case 'f':
				json += '\f';
				break;
			case 'n':
				json += '\n';
				break;
			case 't':
				json += '\t';
				break;
			case 'r':
				json += '\r';
				break;
			case '\"':
				json += '\"';
				break;
			case '\\':
				json += '\\';
				break;
			default: [[unlikely]]
				Nork::Logger::Error("Unrecognised escaped character: \"", formatted[i + 1], "\"");
			}
			i++; // skip escaped character
		}
		else [[likely]]
		{
			json += c;
		}
	}
	return json;
}

std::string JsonObject::ToString() const
{
	if (properties.empty())
		return "{}";
	std::stringstream ss;

	ss << "{";
	for (size_t i = 0; i < order.size(); i++)
	{
		ss << "\"" << order[i] << "\"" << ":" << properties.at(order[i]) << ",";
	}
	ss.seekp(-1, ss.cur) << "}";
	return ss.str();
}

std::string JsonObject::ToStringFormatted() const
{
	return FormatJsonString(ToString());
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
static size_t FindClosing(char open, char close, const std::string& str, size_t idx)
{
	int depth = 1;
	while (++idx < str.size())
	{
		char c = str[idx];
		if (c == '\"') // skip strings
		{
			do
				idx = str.find_first_of('\"', idx + 1); // jump to end of string
			while (str[idx - 1] == '\\'); // escaped double-quotes are still inside string
		}
		else if (c == open)
		{
			depth++;
		}
		else if (c == close)
		{
			if (--depth == 0)
			{
				return idx;
			}
		}
	}
}
static std::string GetNextPropertyValue(const std::string& json, size_t& pos)
{
	static const std::string validArithmeticStartRange = "-0123456789";
	static const std::string validArithmeticEndRange = "0123456789.e-";
	size_t end;
	std::string value;
	if (json[pos] == '\"')
	{
		end = json.find_first_of('\"', pos + 1); // not handling \" being in it
		value = json.substr(pos, ++end - pos);
	}
	else if (validArithmeticStartRange.find(json[pos]) != json.npos)
	{
		end = json.find_first_not_of(validArithmeticEndRange, pos + 1);
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
		value = json.substr(pos, ++end - pos);
	}
	else if (json[pos] == '[')
	{
		end = FindClosing('[', ']', json, pos);
		value = json.substr(pos, ++end - pos);
	}
	pos = end;
	return value;
}
JsonObject JsonObject::Parse(const std::string& json)
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

JsonObject JsonObject::ParseFormatted(const std::string& json)
{
	return Parse(UnformatJsonString(json));
}

JsonArray JsonArray::Parse(const std::string& json)
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
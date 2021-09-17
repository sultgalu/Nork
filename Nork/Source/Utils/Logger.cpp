#include "Logger.h"

namespace Nork
{
	inline bool operator&(Logger::Level t1, Logger::Level t2)
	{
		return (static_cast<int>(t1) & static_cast<int>(t2));
	}

	const char* Logger::GetLevelName(Logger::Level level) const
	{
		switch (level)
		{
		case Logger::Level::None:
			return "None";
		case Logger::Level::Info:
			return "Info";
		case Logger::Level::Warning:
			return "Warning";
		case Logger::Level::Error:
			return "Error";
		case Logger::Level::Debug:
			return "Debug";
		default:
			return "UNEXPECTED_LEVEL";
		}
	}

	const char* Logger::GetCurrentTimeFormatted() const
	{
		return "12:30";
	}

	Logger Logger::Info = Logger(Level::Info);
	Logger Logger::Warning = Logger(Level::Warning);
	Logger Logger::Error = Logger(Level::Error);
	Logger Logger::Debug = Logger(Level::Debug);

	void Logger::PushStream(std::ostream& str, Level levels)
	{
		if (levels & Level::Info)
			Info.streams.push_back(&str);
		if (levels & Level::Warning)
			Warning.streams.push_back(&str);
		if (levels & Level::Error)
			Error.streams.push_back(&str);
		if (levels & Level::Debug)
			Debug.streams.push_back(&str);
	}
}

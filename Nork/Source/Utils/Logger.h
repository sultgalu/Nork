#pragma once

namespace Nork
{
	class Logger
	{
	public:
		enum class Level : int
		{
			None = 0, Info = 1 << 1, Warning = 1 << 2, Error = 1 << 3, Debug = 1 << 4
		};

		static void PushStream(std::ostream& str, Level _for = (Level)((1 << 5) - 1));

		template<typename... T>
		void operator()(T... args) const
		{
			Log(args...);
		}

		template<typename... T>
		void Log(T... args) const
		{
			for (int i = 0; i < streams.size(); i++)
			{
				*streams[i] << GetLevelName(level) << "(" << GetCurrentTimeFormatted() << "): ";
			}
			_Log(args...);
		}

		static Logger Error;
		static Logger Warning;
		static Logger Info;
		static Logger Debug;

	private:
		Logger(Level level) : level(level) {}
	private:
		std::vector<std::ostream*> streams;
		Level level = Level::None;
	private:
		const char* GetLevelName(Logger::Level level) const;
		const char* GetCurrentTimeFormatted() const;

		// Helper functions for templated functionality
		template<typename T, typename... R>
		inline void _Log(T arg, R... args) const
		{
			for (size_t j = 0; j < streams.size(); j++)
			{
				*streams[j] << arg;
			}
			_Log(args...);
		}
		template<typename T>
		inline void _Log(T arg) const
		{
			for (size_t j = 0; j < streams.size(); j++)
			{
				*streams[j] << arg << "\n";
			}
		}
		inline void _Log() const
		{
		}
	};

	struct MetaLogger
	{
		MetaLogger(std::source_location src = std::source_location::current()) : src(src) {}

		template<typename... T>
		void Warning(T... args)
		{
			Log(Logger::Warning, args...);
		}

		template<typename... T>
		void Error(T... args)
		{
			Log(Logger::Error, args...);
		}

		template<typename... T>
		void Debug(T... args)
		{
			Log(Logger::Debug, args...);
		}
	private:
		template<typename... T>
		void Log(Logger& logger, T... args)
		{
			logger(args..., "\n\t", src.file_name(), " -> ", src.function_name(), " [", src.line(), ":", src.column(), "]\n");
		}
		std::source_location src;
	};


	inline Logger::Level operator|(Logger::Level t1, Logger::Level t2)
	{
		return static_cast<Logger::Level>(static_cast<int>(t1) | static_cast<int>(t2));
	}
}


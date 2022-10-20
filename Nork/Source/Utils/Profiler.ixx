export module Nork.Utils:Profiler;

import :Timer;

export namespace Nork
{
	class Profiler
	{
	public:
		struct Entry
		{
			float expected, actual;
			std::vector<std::string> scopes;
			std::source_location srcLoc;
		};
		struct Node
		{
			std::string_view scope;
			Entry* entry;
			std::vector<Node*> childs;

			~Node()
			{
				for (size_t i = 0; i < childs.size(); i++)
				{
					delete childs[i];
				}
			}
		};
		static void Clear()
		{
			entries.clear();
		}
		static Node& GetTree()
		{
			return root;
		}
		static void GenerateTree();

		struct Base
		{
			Timer t;
			Entry entry;
		};

		class Basic : Base
		{
		public:
			Basic(Entry entry) : Base({ Timer(), entry }) {}
			Basic& Start()
			{
				t = Timer();
				return *this;
			}
			void Stop()
			{
				entry.actual = t.Elapsed();
				entries.push_back(entry);
			}
		};

		class Scoped : Basic
		{
		public:
			Scoped(Entry entry) : Basic(entry)
			{
				Start();
			}
			~Scoped()
			{
				Stop();
			}
		};

		template<class... T>
		static Basic GetProfiler(float expect, T... scopeArgs)
		{
			std::vector<std::string> scopes;
			return Basic(Entry{
				.expected = expect,
				.scopes = GetScopes(scopes, scopeArgs...)
				});
		}

		template<class T, class... Rest>
		static std::vector<std::string>& GetScopes(std::vector<std::string>& scopes, T scope, Rest... rest)
		{
			scopes.push_back(scope);
			if constexpr (sizeof...(Rest) > 0)
			{
				GetScopes(scopes, rest...);
			}
			return scopes;
		}

		struct ProfilerBuilder
		{
			ProfilerBuilder(std::source_location loc = std::source_location::current())
			{
				entry.srcLoc = loc;
			}
			ProfilerBuilder& AppendScope(std::string str)
			{
				entry.scopes.push_back(str);
				return *this;
			}
			ProfilerBuilder& ExpectMillis(float val)
			{
				entry.expected = val;
				return *this;
			}
			Basic GetProfiler()
			{
				return Basic(entry);
			}
			Scoped GetScopedProfiler()
			{
				return Scoped(entry);
			}
			Entry entry;
		};
	private:
		inline static Node root;
		inline static std::vector<Entry> entries;
	};

	enum class Categories
	{
		Graphics, Physics, Core
	};
}
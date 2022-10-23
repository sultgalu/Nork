export module Nork.std;

//------------UTILITIES--------------

export import <any>;
export import <bitset>;
export import <chrono>;
export import <functional>;
export import <optional>;
export import <source_location>;
export import <tuple>;
export import <typeinfo>;
export import <typeindex>;

export import <memory>;

export import <limits>;
//------------STRINGS--------------

export import <format>;
export import <string>;
export import <string_view>;
export import <format>;

//-----------CONTAINERS------------

export import <map>; // multimap ->; instead of map<key,vector<val>;>;, map ->; use comperator to sort a map
export import <queue>; // use for plots
export import <stack>;
export import <unordered_map>;
export import <unordered_set>;
export import <vector>;
export import <array>;
export import <span>;

//-------------RANGES--------------

export import <ranges>;

//----------ALGORITHMS-------------

export import <algorithm>;

//-----------NUMERICS--------------

export import <bit>;
export import <numbers>;
export import <random>;
export import <ratio>;

//-----------INPUT-OUTPUT---------

export import <fstream>;
export import <iostream>;
export import <sstream>; // for logging to a string in memory (Editor)

//-----------FILESYSTEM-----------

export import <filesystem>;

//-----------THREADING-----------

export import <thread>;
export import <mutex>;
export import <semaphore>;
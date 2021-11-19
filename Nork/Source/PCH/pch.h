#pragma once

//------------UTILITIES--------------

#include <any>
#include <bitset>
#include <chrono>
#include <functional>
#include <optional>
#include <source_location>
#include <tuple>
#include <typeinfo>
#include <typeindex>

#include <memory>

#include <limits>
//------------STRINGS--------------

#include <format>
#include <string>
#include <string_view>
#include <format>

//-----------CONTAINERS------------

#include <map> // multimap -> instead of map<key,vector<val>>, map -> use comperator to sort a map
#include <queue> // use for plots
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <span>

//-------------RANGES--------------

#include <ranges>

//----------ALGORITHMS-------------

#include <algorithm>

//-----------NUMERICS--------------

#include <bit>
#include <numbers>
#include <random>
#include <ratio>

//-----------INPUT-OUTPUT---------

#include <fstream>
#include <iostream>
#include <sstream> // for logging to a string in memory (Editor)

//-----------FILESYSTEM-----------

#include <filesystem>

////-----------------OPENGL-----------------------

#include <glad/glad.h>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/quaternion.hpp>

#include <glfw/glfw3.h>

////-----------------EDITOR-----------------------

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

////-----------------OTHERS-----------------------

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <entt/single_include/entt/entt.hpp>

//--------------OURS----------------

#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Utils/Template.h"
#include "Utils/Profiler.h"
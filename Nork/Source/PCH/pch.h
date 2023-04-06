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
#include <execution>

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
namespace fs = std::filesystem;

//-----------THREADING-----------

#include <thread>
#include <mutex>
#include <semaphore>

////-----------------VULKAN/NO OPENGL-----------------------

// #include <glad/glad.h>
// #define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <vulkan/vulkan.hpp>
// #define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_format_traits.hpp>
//#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

////-----------------IMGUI-----------------------

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

////-----------------OTHERS-----------------------

#include <entt/entt.hpp>

//--------------OURS----------------

#include "Utils/Logger.h"
#include "Utils/Template.h"
#include "Utils/Timer.h"
#include "Utils/Observed.h"
#include "Utils/Json.h"
#include "Utils/FileUtils.h"
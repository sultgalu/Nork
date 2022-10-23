export module Nork.ThirdParty;

export {

	////-----------------OPENGL-----------------------

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glfw/glfw3.h>

////-----------------IMGUI-----------------------

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

////-----------------OTHERS-----------------------

#include <entt/entt.hpp>
}
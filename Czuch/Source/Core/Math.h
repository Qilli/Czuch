#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm.hpp"
#include "../vendors/glm/gtc/matrix_transform.hpp"
#include "../vendors/glm/gtx/quaternion.hpp"
#include "../vendors/glm/gtx/euler_angles.hpp"
#include "../vendors/glm/gtx/transform.hpp"
#include "../vendors/glm/gtc/type_ptr.hpp"
#include "../vendors/glm/vec3.hpp" // glm::vec3
#include "../vendors/glm/vec4.hpp" // glm::vec4
#include "../vendors/glm/mat4x4.hpp" // glm::mat4
#include "../vendors/glm/ext/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
#include "../vendors/glm/ext/matrix_clip_space.hpp" // glm::perspective
#include "../vendors/glm/ext/scalar_constants.hpp" // glm::pi
#include "../vendors/glm/gtx/matrix_decompose.hpp"

namespace Czuch
{
#define Mat4x4 glm::Mat4
#define Vec4 glm::Vec4
#define Vec3 glm::vec3
#define Vec2 glm::vec2

#define EPSILON 0.001f
#define PI glm::pi<float>()
#define RAD2DEG 180.0f / PI

#define Vec3ToString(vec) std::string(std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z))

}


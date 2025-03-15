#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/ext/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
#include "glm/ext/matrix_clip_space.hpp" // glm::perspective
#include "glm/ext/scalar_constants.hpp" // glm::pi
#include "glm/gtx/matrix_decompose.hpp"

namespace Czuch
{
#define Mat4x4 glm::mat4
#define Mat3x3 glm::mat3
#define Vec4 glm::vec4
#define Vec3 glm::vec3
#define Vec2 glm::vec2
#define Color Vec4

#define EPSILON 0.001f
#define PI glm::pi<float>()
#define RAD2DEG 180.0f / PI

#define Vec3ToString(vec) std::string(std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z))


	struct Colors
	{
	public:
		static Color White;
	};
}


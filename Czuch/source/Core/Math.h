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
#define DEG2RAD PI / 180.0f
#define UP Vec3(0.0f, 1.0f, 0.0f)
#define RIGHT Vec3(1.0f, 0.0f, 0.0f)
#define FORWARD Vec3(0.0f, 0.0f,1.0f)

#define Vec3ToString(vec) std::string(std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z))


	struct CZUCH_API Colors
	{
	public:
		static Color White;
		static Color Green;
		static Color Yellow;
		static Color Red;
	};

	struct AABB
	{
	public:
		Vec3 min;
		Vec3 max;

		AABB() : min(0.0f), max(0.0f) {}
		AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}
		AABB(const AABB& other) : min(other.min), max(other.max) {}

		Vec3 GetCenter() const
		{
			return (min + max) * 0.5f;
		}

		Vec3 GetSize() const
		{
			return max - min;
		}

		Vec3 GetHalfSize() const
		{
			return GetSize() * 0.5f;
		}

		AABB GetScaled(float scale) const
		{
			Vec3 center = GetCenter();
			Vec3 halfSize = GetHalfSize() * scale;
			return AABB(center - halfSize, center + halfSize);
		}
	};

	struct OBB
	{
		Vec3 center;
		Mat4x4 orientation;
		Vec3 extents;// Half-size of the box along its local axes

		OBB() : center(0.0f), orientation(Mat4x4(1.0f)), extents(0.0f) {} 
		OBB(const Vec3& center, const Mat4x4& orientation, const Vec3& extents)
			: center(center), orientation(orientation), extents(extents) {}


		/// <summary>
		/// input should be in range [-1,1]
		/// </summary>
		/// <param name="localPoint"></param>
		/// <returns></returns>
		Vec3 TransformLocalPoint(const Vec3& localPoint) const
		{
			return Vec3(orientation * Vec4(localPoint * extents, 1.0f)) + center;
		}
	};

	OBB TransformAABBToOBB(const AABB& local_aabb, const Mat4x4& localToWorld);

	/// <summary>
	/// Compute a new orientation matrix for a given world center and normal vector.
	/// This function is useful for aligning an object to a surface normal in world space.
	/// The result is new world space where normal is forward direction and right, are tangents
	/// </summary>
	/// <param name="worldCenter"></param>
	/// <param name="normal"></param>
	/// <returns></returns>
	Mat3x3 GetNewSpaceOrientation(const Vec3& worldCenter, const Vec3& normal,const Vec3 inUP=UP);


	inline glm::vec3 GammaToLinear(const glm::vec3& colorGamma)
	{
		return glm::pow(colorGamma, glm::vec3(2.2f));
	}

	inline glm::vec3 LinearToGamma(const glm::vec3& colorLinear)
	{
		return glm::pow(colorLinear, glm::vec3(1.0f / 2.2f));
	}

	inline glm::vec4 GammaToLinear(const glm::vec4& colorGamma)
	{
		glm::vec3 rgbLinear = glm::pow(glm::vec3(colorGamma), glm::vec3(2.2f));
		return glm::vec4(rgbLinear, colorGamma.a);
	}

	inline glm::vec4 LinearToGamma(const glm::vec4& colorLinear)
	{
		glm::vec3 rgbGamma = glm::pow(glm::vec3(colorLinear), glm::vec3(1.0f / 2.2f));
		return glm::vec4(rgbGamma, colorLinear.a);
	}

	struct Viewport
	{
		float x, y, width, height;// 0.0f - 1.0f range
		Viewport(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f)
			: x(x), y(y), width(width), height(height) {}
	};
}


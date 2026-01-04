#pragma once
#include"Core/Math.h"
#include"../Subsystems/Assets/Asset/Asset.h"

namespace Czuch
{
	struct LineInstanceData {
		Vec4 posA;
		Vec4 posB;
		Vec4 color;

		LineInstanceData(const Vec3& start, const Vec3& end, const Color& color)
			: posA(start.x,start.y,start.z,1.0f), posB(end.x,end.y,end.z,1.0f), color(GammaToLinear(color)) {}
	};

	struct PointInstanceData {
		Vec4 position;
		Vec4 color;

		PointInstanceData(const Vec3& point, const Color& color,const float inSize)
			: position(point.x,point.y,point.z,inSize), color(GammaToLinear(color)) {}
	};

	struct TriangleInstanceData {
		Vec4 v1;
		Vec4 v2;
		Vec4 v3;
		Vec4 color;

		TriangleInstanceData(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color)
			: v1(a.x,a.y,a.z,1.0f), v2(b.x,b.y,b.z,1.0f), v3(c.x,c.y,c.z,1.0f), color(GammaToLinear(color)) {}
	};

	struct MeshInstanceData {
		AssetHandle mesh;
		Mat4x4 transform;
		Vec4 color;

		MeshInstanceData(const AssetHandle& mesh, const Mat4x4& transform, const Color& color)
			: mesh(mesh), transform(transform), color(GammaToLinear(color)) {}
	};



	class IDebugDrawBuilder
	{
	public:
		virtual ~IDebugDrawBuilder() = default;
		virtual void DrawLine(const Vec3& start, const Vec3& end, const Color& color) = 0;
		virtual void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color) = 0;
		virtual void DrawPoint(const Vec3& point, const Color& color,const float size) = 0;
		virtual void DrawQuad(const Vec3& center, const Vec3& normal, float size, const Color& color) = 0;
		virtual void DrawMesh(const AssetHandle mesh, const Mat4x4& transform, const Color& color) = 0;
		virtual void DrawCircle(const Vec3& center,const Vec3& normal, float radius, const Color& color) = 0;
		virtual void DrawLinesSphere(const Vec3& center, float radius, const Color& color) = 0;
		virtual void DrawCone(const Vec3& position, const Vec3& direction, float range, float angle, const Color& color) = 0;
		virtual void DrawLinesList(const Array<Vec3>& points, const Color& color) = 0;
	};
}

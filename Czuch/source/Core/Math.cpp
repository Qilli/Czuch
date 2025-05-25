#pragma once
#include"czpch.h"
#include"Math.h"

namespace Czuch
{
	Color Colors::White = Color(1.0f, 1.0f, 1.0f, 1.0f);
	Color Colors::Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
	Color Colors::Yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
	OBB TransformAABBToOBB(const AABB& local_aabb, const Mat4x4& localToWorld)
	{
		// 1. Get the center and half-extents of the local AABB
		Vec3 local_center = local_aabb.GetCenter();
		Vec3 local_half_extents = local_aabb.GetHalfSize();

		// 2. Transform the local center to get the world space OBB center
		// Assuming Mat4x4 * Vec3 performs point transformation (includes translation)
		Vec3 world_obb_center = localToWorld * Vec4(local_center.x,local_center.y,local_center.z,1.0f);

		// 3. Determine the orientation and scaled extents of the OBB in world space.
		// The columns of the localToWorld matrix (excluding translation) represent
		// the transformed local basis vectors, scaled by the matrix's scale.
		Vec3 transformed_axis_x = Vec3(localToWorld[0][0], localToWorld[1][0], localToWorld[2][0]);
		Vec3 transformed_axis_y = Vec3(localToWorld[0][1], localToWorld[1][1], localToWorld[2][1]);
		Vec3 transformed_axis_z = Vec3(localToWorld[0][2], localToWorld[1][2], localToWorld[2][2]);

		// The OBB's half-extents in world space along its *oriented* axes are the
		// local half-extents scaled by the matrix's scale along each axis.
		// We can represent the OBB with its world center, world orientation (rotation),
		// and *local* half-extents. The scaling is implicitly handled by the orientation matrix
		// when the OBB is used. Alternatively, we can store the world-scaled half-extents
		// and a pure rotation matrix for orientation. Storing the local half-extents
		// and the rotation part of the matrix is common.

		// Extract the rotation part from the localToWorld matrix.
		// This is the top-left 3x3 part of the matrix. If the matrix includes non-uniform
		// scaling or shear, simply taking the top-left 3x3 might not result in a pure rotation
		// matrix. A robust solution involves orthonormalizing the basis vectors if you need
		// a pure rotation and applying the scale to the extents. For a simple case
		// (translation, rotation, uniform scale), the top-left 3x3 is sufficient for orientation.

		Mat4x4 world_obb_orientation;
		world_obb_orientation[0][0] = localToWorld[0][0]; world_obb_orientation[0][1] = localToWorld[0][1]; world_obb_orientation[0][2] = localToWorld[0][2]; world_obb_orientation[0][3] = 0.0f;
		world_obb_orientation[1][0] = localToWorld[1][0]; world_obb_orientation[1][1] = localToWorld[1][1]; world_obb_orientation[1][2] = localToWorld[1][2]; world_obb_orientation[1][3] = 0.0f;
		world_obb_orientation[2][0] = localToWorld[2][0]; world_obb_orientation[2][1] = localToWorld[2][1]; world_obb_orientation[2][2] = localToWorld[2][2]; world_obb_orientation[2][3] = 0.0f;
		world_obb_orientation[3][0] = 0.0f;            world_obb_orientation[3][1] = 0.0f;            world_obb_orientation[3][2] = 0.0f;            world_obb_orientation[3][3] = 1.0f;

		//extract the scale from the localToWorld matrix
		Vec3 scale;
		scale.x = glm::length(transformed_axis_x);
		scale.y = glm::length(transformed_axis_y);
		scale.z = glm::length(transformed_axis_z);

		// Normalize the rotation part to ensure it's a valid rotation matrix.
		world_obb_orientation[0] = glm::normalize(world_obb_orientation[0]);
		world_obb_orientation[1] = glm::normalize(world_obb_orientation[1]);
		world_obb_orientation[2] = glm::normalize(world_obb_orientation[2]);

		Vec3 world_obb_extents = local_half_extents*scale; // The extents remain the local half-sizes

		return OBB(world_obb_center, world_obb_orientation, world_obb_extents);
	}
}
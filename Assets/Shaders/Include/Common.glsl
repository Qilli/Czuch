struct RenderObject {
        mat4 localToWorldTransformation;
		mat4 invTransposeToWorldMatrix;
		ivec4 materialIndexAndFlags; // x: material index, y: flags (e.g., 0x1 for double-sided)
 };

layout(std430,set =0, binding = 1) readonly buffer RenderObjectsData {
	RenderObject renderObjects[];
};

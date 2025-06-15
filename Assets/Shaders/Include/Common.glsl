struct RenderObject {
        mat4 localToWorldTransformation;
		mat4 invTransposeToWorldMatrix;
 };

layout(std430,set =0, binding = 1) readonly buffer RenderObjectsData {
	RenderObject renderObjects[];
};

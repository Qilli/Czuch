#pragma once
#include"Renderer/RenderPassControl.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"UITextureSource.h"

namespace Czuch
{
	class VulkanRenderer;
	class CommandBuffer;
	class Camera;
	class VulkanDevice;

	class VulkanRenderPassControlBase : public RenderPassControl
	{
	public:
		VulkanRenderPassControlBase(VulkanDevice* device, VulkanRenderer* renderer, Camera* cam, U32 width, U32 height, RenderPassType type, bool handleWindowResize);
		virtual ~VulkanRenderPassControlBase();
		void BindInputTextures(GraphicsDevice* device, FrameGraphNode* node) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void* GetRenderPassResult() override;
		void TransitionResultsToShaderReadOnly(CommandBuffer* cmd) override;
		void SetAsTextureSource() override;
		void Release() override;
		void ReleaseDependencies() override;
		void Resize(int width, int height) override;
		void Init() override;
	protected:
		VulkanRenderer* m_Renderer;
		VulkanDevice* m_Device;
		UITextureSource* m_TextureSource;
	};
}
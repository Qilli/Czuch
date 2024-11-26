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
		void PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void* GetRenderPassResult() override;
		void SetAsTextureSource() override;
		void Release() override;
		void Resize(int width, int height) override;
	protected:
		VulkanRenderer* m_Renderer;
		VulkanDevice* m_Device;
		UITextureSource* m_TextureSource;
	};
}
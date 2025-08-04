#include"czpch.h"
#include"VulkanMainRenderPass.h"
#include"Renderer/Vulkan/VulkanRenderer.h"
#include"Renderer/Vulkan/VulkanCommandBuffer.h"
#include"Renderer/Vulkan/DescriptorAllocator.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"backends/imgui_impl_vulkan.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include"../VulkanDevice.h"


namespace Czuch
{
	VulkanMainRenderPass::VulkanMainRenderPass(VulkanDevice* device, VulkanRenderer* renderer) : VulkanRenderPassControlBase(device,renderer,nullptr,0, 0, RenderPassType::Final,true)
	{
		SetPriority(0);
		INVALIDATE_HANDLE(m_FinalTexture);
	}

	void VulkanMainRenderPass::PreDraw(CommandBuffer* cmd,Renderer* renderer)
	{
		RenderPassControl::PreDraw(cmd, renderer);
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		int imageIndex = m_Device->GetCurrentImageIndex();

		cmdBuffer->SetClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		cmdBuffer->SetDepthStencil(1.0f, 0);


		if (EngineRoot::GetEngineSettings().dynamicRendering)
		{
			m_Device->TransitionSwapChainImageLayoutPreDraw(cmdBuffer, imageIndex);
			m_Device->StartDynamicRenderPass(cmdBuffer, imageIndex);
		}
		else
		{
			m_Device->BindSwapChainRenderPass(cmdBuffer, imageIndex);
		}

		ViewportDesc vpdesc{};
		vpdesc.x = m_Viewport.x;
		vpdesc.y = m_Viewport.y;
		vpdesc.minDepth = 0.0f;
		vpdesc.maxDepth = 1.0f;
		vpdesc.width = (U32)m_Device->GetSwapchainWidth()*m_Viewport.width;
		vpdesc.height = (U32)m_Device->GetSwapchainHeight()*m_Viewport.height;
		cmdBuffer->SetViewport(vpdesc);

		ScissorsDesc scissors{};
		scissors.offsetX = 0;
		scissors.offsetY = 0;
		scissors.width = (U32)m_Device->GetSwapchainWidth();
		scissors.height = (U32)m_Device->GetSwapchainHeight();
		cmdBuffer->SetScrissors(scissors);
	}

	void VulkanMainRenderPass::PostDraw(CommandBuffer* cmd,Renderer* renderer)
	{
		RenderPassControl::PostDraw(cmd, renderer);
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;

		if (EngineRoot::GetEngineSettings().dynamicRendering)
		{
			cmdBuffer->EndDynamicRenderPassForMainPass();
			m_Device->TransitionSwapChainImageLayoutPostDraw(cmdBuffer, m_Device->GetCurrentImageIndex());
		}
		else
		{
			cmdBuffer->EndCurrentRenderPass();
		}
	}


	void VulkanMainRenderPass::Execute(CommandBuffer* cmd)
	{
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		if (EngineRoot::GetEngineSettings().engineMode == EngineMode::Editor)
		{
			//m_Renderer->DrawScene((VulkanCommandBuffer*)cmdBuffer);
			m_Device->DrawUI(cmdBuffer);
		}
		else
		{
			m_Renderer->DrawFullScreenQuad((VulkanCommandBuffer*)cmdBuffer, DefaultAssets::FINAL_PASS_MATERIAL_INSTANCE);
			m_Device->DrawUI(cmdBuffer);
		}
	}


	void VulkanMainRenderPass::SetFinalTexture(CommandBuffer* cmd,TextureHandle texture)
	{
		bool setForMaterial =!HANDLE_IS_VALID(m_FinalTexture);
		m_FinalTexture = texture;
		m_Device->TryTransitionImageLayout(cmd,m_FinalTexture, ImageLayout::SHADER_READ_ONLY_OPTIMAL, 0, 1);
		if (setForMaterial)
		{
			MaterialInstance* mat = m_Device->AccessMaterialInstance(DefaultAssets::FINAL_PASS_MATERIAL_INSTANCE);
			mat->params[0].SetSampler(0,texture,0);
		}
	}

	void VulkanMainRenderPass::SetViewportAndScissor(Camera* camera)
	{
		m_Viewport = camera->GetViewport();
	}

	void* VulkanMainRenderPass::GetFinalTexture()
	{
		auto texture = m_Device->AccessTexture(m_FinalTexture);
		auto vulkanTexture = Internal_to_Texture(texture);
		return vulkanTexture->imageView;
	}

}	
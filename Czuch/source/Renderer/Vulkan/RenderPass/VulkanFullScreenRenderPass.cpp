#include"czpch.h"
#include"VulkanFullScreenRenderPass.h"
#include"Renderer/Vulkan/VulkanRenderer.h"
#include"Renderer/Vulkan/VulkanCommandBuffer.h"
#include"Renderer/Vulkan/DescriptorAllocator.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"backends/imgui_impl_vulkan.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include"../VulkanDevice.h"
#include"Subsystems/Assets/AssetsManager.h"
#include"Subsystems/Assets/Asset/ShaderAsset.h"
#include"Subsystems/Assets/Asset/MaterialAsset.h"
#include"Subsystems/Assets/Asset/MaterialInstanceAsset.h"

namespace Czuch
{
	VulkanFullScreenRenderPass::VulkanFullScreenRenderPass(VulkanDevice* device, VulkanRenderer* renderer) : VulkanRenderPassControlBase(device, renderer, nullptr, 0, 0, RenderPassType::FullScreenPass, true)
	{
		SetPriority(0);
		INVALIDATE_HANDLE(m_SourceTexture);
		INVALIDATE_HANDLE(m_TargetTexture);
	}

	void VulkanFullScreenRenderPass::PreDraw(CommandBuffer* cmd, Renderer* renderer)
	{
		RenderPassControl::PreDraw(cmd, renderer);
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		int imageIndex = m_Device->GetCurrentImageIndex();

		cmdBuffer->SetClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		cmdBuffer->SetDepthStencil(1.0f, 0);
		cmdBuffer->BindPass(m_RenderPassHandle, m_FramebufferHandle);


		ViewportDesc vpdesc{};
		vpdesc.x = m_Viewport.x;
		vpdesc.y = m_Viewport.y;
		vpdesc.minDepth = 0.0f;
		vpdesc.maxDepth = 1.0f;
		vpdesc.width = (U32)m_Device->GetSwapchainWidth() * m_Viewport.width;
		vpdesc.height = (U32)m_Device->GetSwapchainHeight() * m_Viewport.height;
		cmdBuffer->SetViewport(vpdesc);

		ScissorsDesc scissors{};
		scissors.offsetX = 0;
		scissors.offsetY = 0;
		scissors.width = (U32)m_Device->GetSwapchainWidth();
		scissors.height = (U32)m_Device->GetSwapchainHeight();
		cmdBuffer->SetScrissors(scissors);
	}

	void VulkanFullScreenRenderPass::PostDraw(CommandBuffer* cmd, Renderer* renderer)
	{
		RenderPassControl::PostDraw(cmd, renderer);
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		cmdBuffer->EndCurrentRenderPass();
	}


	void VulkanFullScreenRenderPass::Execute(CommandBuffer* cmd)
	{
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		m_Renderer->DrawFullScreenQuad((VulkanCommandBuffer*)cmdBuffer, m_MaterialInstanceHandle);
	}


	void VulkanFullScreenRenderPass::SetSourceTexture(CommandBuffer* cmd, TextureHandle texture, TextureHandle depthTexture, RenderPassHandle renderPass)
	{
		if (m_SourceTexture.handle != texture.handle || m_DepthTexture.handle != depthTexture.handle)
		{
			m_SourceTexture = texture;
			m_DepthTexture = depthTexture;
			ReleaseRenderPassAndFrameBuffer();
			ReleaseMaterial();

			CreateRenderPassAndFrameBuffer(renderPass);
			CreateMaterial();
		}

		m_SourceTexture = texture;
		m_DepthTexture = depthTexture;
		m_Device->TryTransitionImageLayout(cmd, m_SourceTexture, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, 0, 1);
	}

	void VulkanFullScreenRenderPass::SetTargetTexture(CommandBuffer* cmd, TextureHandle texture)
	{
		m_TargetTexture = texture;
		if (HANDLE_IS_VALID(m_TargetTexture))
		{
			m_Device->TryTransitionImageLayout(cmd, m_TargetTexture, ImageLayout::SHADER_READ_ONLY_OPTIMAL, 0, 1);
			MaterialInstance* mat = m_Device->AccessMaterialInstance(m_MaterialInstanceHandle);
			mat->params[0].SetSampler(0, m_TargetTexture);
		}
	}

	void VulkanFullScreenRenderPass::SetViewportAndScissor(Camera* camera)
	{
		m_Viewport = camera->GetViewport();
	}

	void VulkanFullScreenRenderPass::Release()
	{
		ReleaseMaterial();
		ReleaseRenderPassAndFrameBuffer();
		VulkanRenderPassControlBase::Release();
		ReleaseRenderPassAndFrameBuffer();
	}

	void VulkanFullScreenRenderPass::ReleaseRenderPassAndFrameBuffer()
	{
		if (HANDLE_IS_VALID(m_RenderPassHandle))
		{
			m_Device->Release(m_RenderPassHandle);
			INVALIDATE_HANDLE(m_RenderPassHandle);
		}
		if (HANDLE_IS_VALID(m_FramebufferHandle))
		{
			m_Device->Release(m_FramebufferHandle);
			INVALIDATE_HANDLE(m_FramebufferHandle);
		}
	}

	void VulkanFullScreenRenderPass::CreateRenderPassAndFrameBuffer(RenderPassHandle handle)
	{
		if (HANDLE_IS_VALID(m_RenderPassHandle) || HANDLE_IS_VALID(m_FramebufferHandle))
		{
			return; // Already created
		}

		auto renderPass = m_Device->AccessRenderPass(handle);
		auto colorAttachment = renderPass->desc.colorAttachments[0];

		RenderPassDesc desc{};
		desc.AddAttachment(colorAttachment.format, colorAttachment.layout, AttachmentLoadOp::LOAD);
		desc.depthLoadOp = renderPass->desc.depthLoadOp;
		desc.depthStencilFormat = renderPass->desc.depthStencilFormat;
		desc.depthLoadOp = renderPass->desc.depthLoadOp;
		desc.type = RenderPassType::FullScreenPass;

		m_RenderPassHandle = m_Device->CreateRenderPass(&desc);

		FrameBufferDesc fbDesc{};
		fbDesc.renderPass = m_RenderPassHandle;
		fbDesc.width = (U32)m_Device->GetSwapchainWidth();
		fbDesc.height = (U32)m_Device->GetSwapchainHeight();
		fbDesc.AddRenderTexture(m_SourceTexture);
		fbDesc.SetDepthStencilTexture(m_DepthTexture);


		m_FramebufferHandle = m_Device->CreateFrameBuffer(&fbDesc);
	}

	void VulkanFullScreenRenderPass::CreateMaterial()
	{
		// fullscreen pass material
		auto finalVS = AssetsManager::Get().LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\VertexFinalPassShader.vert", {});
		auto finalPS = AssetsManager::Get().LoadAsset<ShaderAsset, LoadSettingsDefault>("Shaders\\FragmentFinalPassShader.frag", {});

		MaterialPassDesc desc;
		desc.vs = finalVS;
		desc.ps = finalPS;
		desc.pt = PrimitiveTopology::TRIANGLELIST;
		desc.rs.cull_mode = CullMode::NONE;
		desc.rs.fill_mode = PolygonMode::SOLID;
		desc.dss.depth_enable = true;
		desc.dss.depth_func = CompFunc::ALWAYS;
		desc.dss.depth_write_mask = DepthWriteMask::ZERO;
		desc.dss.stencil_enable = false;
		desc.bindPoint = BindPoint::BIND_POINT_GRAPHICS;
		desc.passType = RenderPassType::FullScreenPass;

		DescriptorSetLayoutDesc desc_tex{};
		desc_tex.shaderStage = (U32)ShaderStage::PS;
		desc_tex.AddBinding("MainTexture", DescriptorType::SAMPLER, 0, 1, 0, false);

		desc.AddLayout(desc_tex);

		MaterialDefinitionDesc matDesc(1);
		matDesc.EmplacePass(desc);
		matDesc.materialName = "FullScreenPassMaterial";

		MaterialCreateSettings createSettings;
		createSettings.desc = std::move(matDesc);
		m_MaterialAsset = AssetsManager::Get().CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);
		auto materialAsset = AssetsManager::Get().GetAsset<MaterialAsset>(m_MaterialAsset);
		materialAsset->SetPersistentStatus(true);

		MaterialInstanceCreateSettings instanceCreateSettings{};
		instanceCreateSettings.materialInstanceName = "FullScreenPassMaterialInstance";
		instanceCreateSettings.desc.AddSampler("MainTexture", DefaultAssets::WHITE_TEXTURE, false);
		instanceCreateSettings.desc.materialAsset = m_MaterialAsset;
		instanceCreateSettings.desc.isTransparent = false;

		AssetHandle instanceAssetHandle = AssetsManager::Get().CreateAsset<MaterialInstanceAsset, MaterialInstanceCreateSettings>(instanceCreateSettings.materialInstanceName, instanceCreateSettings);
		MaterialInstanceAsset* instanceAsset = AssetsManager::Get().GetAsset<MaterialInstanceAsset>(instanceAssetHandle);
		m_MaterialInstanceHandle = instanceAsset->GetMaterialInstanceResourceHandle();
		m_MaterialInstanceAsset = instanceAssetHandle;
	}

	void VulkanFullScreenRenderPass::ReleaseMaterial()
	{
		if (HANDLE_IS_VALID(m_MaterialInstanceAsset))
		{
			AssetsManager::Get().UnloadAsset<MaterialInstanceAsset>(m_MaterialInstanceAsset);
			INVALIDATE_HANDLE(m_MaterialInstanceHandle);
		}
		if (HANDLE_IS_VALID(m_MaterialAsset))
		{
			AssetsManager::Get().UnloadAsset<MaterialAsset>(m_MaterialAsset);
			INVALIDATE_HANDLE(m_MaterialAsset);
		}
		m_MaterialAsset = {};
		m_MaterialInstanceAsset = {};
	}
	
}	
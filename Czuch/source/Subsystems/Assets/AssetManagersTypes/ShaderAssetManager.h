#pragma once
#include"../AssetsManager.h"
#include "../Asset/ShaderAsset.h"

namespace Czuch
{

	class ShaderAssetManager :public AssetManager
	{
	public:
		ShaderAssetManager(GraphicsDevice* device);
		bool IsFormatSupported(const char* format)const override { return strcmp(format, ".vert") == 0 || strcmp(format, ".frag") == 0; }
		void Init() override;
		int GetAssetType() const override { return (int)AssetType::SHADER; }
		static const CzuchStr& GetShaderIncludePath() { return s_ShaderIncludePath; }
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
		Asset* CreateLoadableAsset(const CzuchStr& path) override;
	private:
		GraphicsDevice* m_Device;
		static CzuchStr s_ShaderIncludePath;
	};

	inline ShaderAssetManager::ShaderAssetManager(GraphicsDevice* device):m_Device(device)
	{
		ShaderAssetManager::s_ShaderIncludePath = AssetsManager::GetStartPath() + "Shaders/Include";
	}

	inline Asset* ShaderAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		ShaderAsset* shaderRes = new ShaderAsset(path,m_Device,(ShaderCreateSettings&)settings,AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, shaderRes);
		return shaderRes;
	}

	inline Asset* ShaderAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		ShaderAsset* shaderRes = new ShaderAsset(path, m_Device,(ShaderLoadSettings&)settings,AssetsManager::GetPtr());
		return shaderRes;
	}

	inline Asset* ShaderAssetManager::CreateLoadableAsset(const CzuchStr& path)
	{
		ShaderLoadSettings settings;
		ShaderAsset* shaderRes = new ShaderAsset(path, m_Device, settings, AssetsManager::GetPtr());
		return shaderRes;
	}

}


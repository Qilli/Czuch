#pragma once
#include "Asset.h"
#include "./Subsystems/Assets/SettingsPerType.h"

namespace Czuch
{
    class GraphicsDevice;
    class MaterialInstanceAssetManager;

	class CZUCH_API MaterialInstanceAsset: public Asset
	{
    public:
        MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceLoadSettings& loadSettings,AssetsManager* assetsManager);
        MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceCreateSettings& settings, AssetsManager* assetsManager);
        ~MaterialInstanceAsset();
        bool LoadAsset() override;
        bool UnloadAsset() override;
        bool CreateFromData() override;
        inline MaterialInstanceHandle GetMaterialInstanceResourceHandle() const { return m_MaterialInstanceResource; }
    public:
        ShortAssetInfo* GetShortAssetInfo() override;
        CzuchStr GetAssetLoadInfo() const override;
        MaterialParamType GetParameterAtIndexType(int index) const;
		bool IsParameterAtIndexInternal(int index) const;
		const CzuchStr& GetParameterAtIndexName(int index) const;
		std::tuple<AssetHandle,I32> GetTextureAssetAtIndex(int index) const;
        std::tuple<Buffer*, UBOLayout*> GetUBOBufferAtIndex(int index) const;
		void UpdateUBOBufferAtIndex(int index);
		void SetTextureParameterAtIndex(int index,AssetHandle asset,I32 resource);
        StorageBufferTagInfo GetInfoDescriptorTag(DescriptorBindingTagType tag,int pass) const;
        void ChangeDataForDescriptorWithTag(DescriptorBindingTagType tag, void* data, U32 size);
		U32 GetParametersCount() const;
    public:
		MaterialInstanceAsset* CloneMaterialInstance();
		void* GetParameterDataWithInfo(StorageBufferTagInfo& info);
        void SetGlobalIndexAndBufferForDescriptorAtIndex(DescriptorBindingTagType tag, I32 index,BufferHandle handle);
		void SetMaterialInstanceAssetManager(MaterialInstanceAssetManager* manager) { m_AssetManager = manager; }
    private:
		void LoadDependencies();
		void UnloadDependencies();
    private:
        GraphicsDevice* m_Device;
		MaterialInstanceDesc m_MaterialInstanceDesc;
        MaterialInstanceHandle m_MaterialInstanceResource;
		MaterialInstance* m_MaterialInstance;
		MaterialInstanceAssetManager* m_AssetManager;
        MaterialInstanceCreateSettings m_MaterialCreateSettings;
        MaterialInstanceLoadSettings m_MaterialLoadSettings;
        Array<char> m_MaterialInstanceBuffer;
		Array<TextureHandle> m_Dependencies;
	};
}



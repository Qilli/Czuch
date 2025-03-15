#pragma once
#include"Subsystems/BaseSubsystem.h"
#include"AssetManager.h"
#include"SettingsPerType.h"
#include<vector>
#include<typeindex>
#include<filesystem>
#include"../Logging.h"


namespace Czuch
{

	class CZUCH_API AssetsManager: public BaseSubsystem<AssetsManager>
	{
	public:
		void Init(RenderSettings* settings) override;
		void Shutdown() override;
		void Update(TimeDelta timeDelta) override;

		virtual void RegisterManager(AssetManager* newMgr,std::type_index type);
		std::filesystem::path GetAssetPath(AssetHandle handle);

		template<class T,class TM>
		AssetHandle LoadAsset(const CzuchStr& path,TM&& settings);

		template<class T, class TM>
		AssetHandle LoadAsset(const CzuchStr& path, TM& settings);

		template<class T,class TM>
		AssetHandle CreateAsset(const CzuchStr& name, TM&& createSettings);

		template<class T, class TM>
		AssetHandle CreateAsset(const CzuchStr& name, TM& createSettings);

		template<class T>
		AssetHandle LoadAsset(const CzuchStr& path);

		template <typename T>
		void UnloadAsset(AssetHandle handle);

		template <typename T>
		void LoadAsset(AssetHandle handle);

		template <typename T>
		T* GetAsset(AssetHandle handle,bool incrementRef=false);

		template<typename T,typename TM>
		T* LoadAndGetResource(const CzuchStr& path, TM&& settings);
		template<typename T, typename TM>
		T* LoadAndGetResource(const StringID& path, TM&& settings);

		template <typename T>
		void IncrementAssetRef(AssetHandle handle);

		void UnloadAndRemoveAsset(const CzuchStr& path);
	public:
		void InitManagers();
	public: 
		const Array<ShortAssetInfo*>& GetAssetsOfTypes(int assetsFilter);
	public:
		int GetAssetsTypeCount() const { return m_AssetsMgrs.size(); }
		auto GetAssetManagerIterator() const { return m_AssetsMgrs.begin(); }
		auto GetAssetManagerEnd() const { return m_AssetsMgrs.end(); }
	public:
		bool IsDuringShutdown() const { return m_isDuringShutdown; }
		bool IsFormatSupported(const char* format) const;
		bool IsFormatAssetOfType(const char* foramt,AssetType type) const;
		void CheckIfAssetExistsAndIfNotCreate(const std::filesystem::path& pathRelative) const;
		static const std::string& GetStartPath() { return m_StartPath; }
	public:
		TextureHandle Load2DTexture(const CzuchStr& path);
		AssetHandle CreateMaterialInstance(MaterialInstanceCreateSettings& settings);
		AssetHandle CreateMaterialInstance(const CzuchStr& matName, AssetHandle materialSource);
	private:
		std::unordered_map<std::type_index,AssetManager*> m_AssetsMgrs;
		static std::string m_StartPath;
		bool m_isDuringShutdown = false;
	private:
		Array<ShortAssetInfo*> m_AssetsInfoTemp;
	};

	template<class T, class TM>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path, TM&& settings)
	{
		if(m_isDuringShutdown)
		{
			return { Invalid_Handle_Id };
		}
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->LoadAsset(path, settings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load resource with path: {0} in manager of type {1}", path, result->first.name());
				return {Invalid_Handle_Id};
			}

			return res->GetHandle();
		}
		return { Invalid_Handle_Id };
	}

	template<class T, class TM>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path, TM& settings)
	{
		if (m_isDuringShutdown)
		{
			return { Invalid_Handle_Id };
		}
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->LoadAsset(path, settings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load resource with path: {0} in manager of type {1}", path, result->first.name());
				return AssetHandle();
			}

			return res->GetHandle();
		}
		return AssetHandle();
	}

	template<class T, class TM>
	inline AssetHandle AssetsManager::CreateAsset(const CzuchStr& name, TM&& createSettings)
	{
		if (m_isDuringShutdown)
		{
			return { Invalid_Handle_Id };
		}
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->CreateAsset(name,createSettings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to create asset with name: {0} in manager of type {1}", name, result->first.name());
				return { Invalid_Handle_Id };
			}

			return res->GetHandle();
		}
		return { Invalid_Handle_Id };
	}

	template<class T, class TM>
	inline AssetHandle AssetsManager::CreateAsset(const CzuchStr& name, TM& createSettings)
	{
		if (m_isDuringShutdown)
		{
			return { Invalid_Handle_Id };
		}
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->CreateAsset(name, createSettings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to create asset with name: {0} in manager of type {1}", name, result->first.name());
				return { Invalid_Handle_Id };
			}

			return res->GetHandle();
		}
		LOG_BE_ERROR("Failed to find manager for asset with name {0}",name);
		return { Invalid_Handle_Id };
	}


	template<class T>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path)
	{
		if (m_isDuringShutdown)
		{
			return { Invalid_Handle_Id };
		}
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			BaseLoadSettings defaultSettings;
			Asset* res = mgr->LoadAsset(path,defaultSettings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load asset with path: {0} in manager of type {1}", path, result->first.name());
				return { Invalid_Handle_Id };
			}

			return res->GetHandle();
		}
		return { Invalid_Handle_Id };
	}

	template<typename T>
	inline void AssetsManager::UnloadAsset(AssetHandle handle)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			mgr->UnloadAsset(handle);
			return;
		}
		LOG_BE_ERROR("Failed to find asset for removal with handle id: {0}", handle.handle);
	}

	template<typename T>
	inline void AssetsManager::LoadAsset(AssetHandle handle)
	{
		if (m_isDuringShutdown)
		{
			return;
		}
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			mgr->LoadAsset(handle);
			return;
		}
		LOG_BE_ERROR("Failed to find asset for loading with handle id: {0}", handle.handle);
	}


	template <typename T>
	T* AssetsManager::GetAsset(AssetHandle handle,bool incrementRef)
	{
		auto result=m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res=mgr->GetAsset(handle);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to find asset with handle id: {0} in manager of type {1}", handle.handle,result->first.name());
				return static_cast<T*>(mgr->GetDefaultAsset());
			}

			if (incrementRef)
			{
				res->IncrementRefCounter();
			}
			return static_cast<T*>(res);
		}
		LOG_BE_ERROR("Failed to find asset with handle id: {0}", handle.handle);
		return nullptr;
	}
	template<typename T, typename TM>
	T* AssetsManager::LoadAndGetResource(const CzuchStr& path, TM&& settings)
	{
		if (m_isDuringShutdown)
		{
			return nullptr;
		}
		AssetHandle asset = LoadAsset<T,TM>(path, settings);
		auto assetObj = GetAsset<T>(asset,true);
		return assetObj;
	}

	template<typename T, typename TM>
	T* AssetsManager::LoadAndGetResource(const StringID& path, TM&& settings)
	{
		if (m_isDuringShutdown)
		{
			return nullptr;
		}
		AssetHandle asset = LoadAsset<T, TM>(path.GetStrName(), settings);
		auto assetObj = GetAsset<T>(asset, true);
		return assetObj;
	}

	template<typename T>
	inline void AssetsManager::IncrementAssetRef(AssetHandle handle)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			mgr->IncrementAssetRef(handle);
			return;
		}
		LOG_BE_ERROR("Failed to find asset for incrementing ref with handle id: {0}", handle.handle);
	}
}


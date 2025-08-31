#pragma once
#include "./Renderer/Graphics.h"
#include"./Core/TreeNode.h"
#include"../Logging.h"

namespace Czuch
{

	struct BaseSettings
	{
		bool permamentAsset;

		BaseSettings()
		{
			permamentAsset = false;
		}
	};

	struct BaseLoadSettings: public BaseSettings
	{
		bool dontLoad = false;
	};

	struct BaseCreateSettings: public BaseSettings
	{

	};

	struct LoadSettingsDefault: BaseLoadSettings
	{

	};

	struct TextureLoadSettings: BaseLoadSettings
	{
		TextureDesc::Type type= TextureDesc::Type::TEXTURE_2D;
		bool isUITexture = false;
		bool isBindlessTexture = true;
	};

	struct TextureCreateSettings: BaseCreateSettings
	{
		TextureDesc::Type type = TextureDesc::Type::TEXTURE_2D;
		U32 width = 128;
		U32 height = 128;
		U16 channels = 4;
		std::vector<unsigned char> colors;
		bool isUITexture = false;

		TextureCreateSettings() = default;

	/*	TextureCreateSettings(TextureCreateSettings&& other) noexcept
		{
			LOG_BE_INFO("TextureCreateSettings: Constructing create texture settings with move constructor");
			*this = std::move(other);
		}*/

		/*TextureCreateSettings& operator=(TextureCreateSettings&& other) noexcept
		{
			LOG_BE_INFO("TextureCreateSettings: Operator = create texture settings with move option");
			if (&other != this)
			{
				this->channels = other.channels;
				this->height = other.height;
				this->width = other.width;
				this->type = other.type;
				this->colors = std::move(other.colors);
			}
			return *this;
		}*/
	};

	struct CZUCH_API MaterialCreateSettings : BaseCreateSettings
	{
		MaterialDefinitionDesc desc;
		MaterialCreateSettings() = default;
	};

	struct MaterialLoadSettings : BaseLoadSettings
	{

	};

	struct CZUCH_API MaterialInstanceCreateSettings : BaseCreateSettings
	{
		MaterialInstanceDesc desc;
		CzuchStr materialInstanceName;

		MaterialInstanceCreateSettings()
		{
			desc.AddBuffer("SceneData", INVALID_HANDLE(BufferHandle));
		}
	};

	struct MaterialInstanceLoadSettings : BaseLoadSettings
	{

	};

	struct ShaderLoadSettings: BaseLoadSettings
	{

	};

	struct ShaderCreateSettings: BaseCreateSettings
	{
		ShaderStage stage;
		ShaderFormat format;
		std::vector<char> shaderCode;

		ShaderCreateSettings() = default;

		/*ShaderCreateSettings(ShaderCreateSettings& other) noexcept
		{
			*this = std::move(other);
		}

		ShaderCreateSettings& operator=(ShaderCreateSettings&& other) noexcept
		{
			if (&other != this)
			{
				this->format = other.format;
				this->stage = other.stage;
				this->shaderCode = std::move(other.shaderCode);
			}
			return *this;
		}*/
	};

	struct MeshTreeNodeElement
	{
		I32 meshIndex;
		Mat4x4 localTransform;

		MeshTreeNodeElement(I32 meshIndex, const Mat4x4& localTransform)
			:meshIndex(meshIndex), localTransform(localTransform)
		{

		}

		MeshTreeNodeElement()
		{
			meshIndex = -1;
			localTransform = Mat4x4(1.0f);
		}
	};

	struct ModelLoadSettings : BaseLoadSettings
	{

	};

	struct SingleMeshData
	{
		MeshData meshData;
		MeshHandle meshHandle;
		MaterialInstanceHandle materialInstanceHandle;
	};

	struct MeshDataContainer
	{
		Array<SingleMeshData> meshesData;
		TreeNode<MeshTreeNodeElement> meshesHierarchy;
		bool inited = false;

		MeshHandle GetMeshHandleAt(U32 index) const
		{
			return meshesData[index].meshHandle;
		}

		MeshHandle GetMeshHandleWithName(const CzuchStr& name) const
		{
			for (const auto& mesh : meshesData)
			{
				if (mesh.meshData.meshName == name)
				{
					return mesh.meshHandle;
				}
			}
			return INVALID_HANDLE(MeshHandle);
		}

		U32 GetMeshesCount() const
		{
			return meshesData.size();
		}

		U32 AddMeshData(SingleMeshData&& meshData,Mat4x4 localTransform,TreeNode<MeshTreeNodeElement>* node)
		{
			meshesData.push_back(std::move(meshData));
			U32 index = meshesData.size() - 1;
			if(node!=nullptr)
			{
				node->SetData(MeshTreeNodeElement(index,localTransform));
			}
			return index;
		}

		void Reserve(U32 size)
		{
			meshesData.reserve(size);
		}

		void Clear()
		{
			meshesData.clear();
		}

	};

	struct ModelCreateSettings : BaseCreateSettings
	{
		Array<MeshData> meshesData;
		TreeNode<MeshTreeNodeElement> meshesHierarchy;
		CzuchStr modelName;
	};


}

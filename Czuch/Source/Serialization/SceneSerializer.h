#pragma once
#include"ISerializer.h"
#include"Subsystems/Scenes/Scene.h"
namespace Czuch
{
	class SceneSerializer : public ISerializationControl
	{
	public:
		SceneSerializer(Scene* scene) :m_Scene(scene) {}
		virtual ~SceneSerializer() = default;
	public:
		virtual bool Serialize(std::string& filepath, bool binary = false) override;
		virtual bool Deserialize(std::string& filepath, bool binary = false) override;
	private:
		Scene* m_Scene;
	};
}


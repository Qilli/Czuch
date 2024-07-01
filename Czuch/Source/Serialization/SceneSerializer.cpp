#include "czpch.h"
#include "SceneSerializer.h"

namespace Czuch
{

	bool SceneSerializer::Serialize(const std::string& filepath, bool binary)
	{
		if (binary)
		{
			return true;
		}
		else
		{
			YAML::Emitter out;
			bool sceneResult= m_Scene->Serialize(out,false);
			if (sceneResult)
			{
				std::ofstream file(filepath);
				if (!file.good())
				{
					LOG_BE_ERROR("Failed to open file: {0}", filepath);
					return false;
				}
				file << out.c_str();
				file.close();
				LOG_BE_INFO("Serialized scene {0} to file: {1}", m_Scene->GetSceneName(), filepath);
				return true;
			}
			else
			{
				LOG_BE_ERROR("Failed to serialize scene {0} to file: {1}",m_Scene->GetSceneName(), filepath);
				return false;
			}
		}
	}

	bool SceneSerializer::Deserialize(const std::string& filepath, bool binary)
	{
		return false;
	}
}


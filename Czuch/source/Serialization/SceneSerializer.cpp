#include "czpch.h"
#include "SceneSerializer.h"

namespace Czuch
{

	bool SceneSerializer::Serialize(std::string& filepath, bool binary)
	{
		//make sure path has correct extension
		size_t result = filepath.find_last_of('.');
		if (result == std::string::npos)
		{
			//add extension
			filepath += ".scene";
		}
		else
		{
			//check if extension is correct
			std::string extension = filepath.substr(result);
			if (extension != ".scene")
			{
				//replace for correct extension
				filepath += ".scene";
			}
		}


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

	bool SceneSerializer::Deserialize(std::string& filepath, bool binary)
	{
		if (binary)
		{
			return true;
		}
		else
		{
			std::ifstream file(filepath);
			if (!file.good())
			{
				LOG_BE_ERROR("Failed to open and process scene file: {0}", filepath);
				return false;
			}

			std::stringstream ss;
			ss << file.rdbuf();
			file.close();

			YAML::Node data = YAML::Load(ss.str());
			if (!data)
			{
				LOG_BE_ERROR("Failed to decode scene file: {0}", filepath);
				return false;
			}

			if (data["Scene"])
			{
				m_Scene->ClearScene();

				bool result=m_Scene->Deserialize(data, false);
				if (!result)
				{
					LOG_BE_ERROR("Failed to deserialize scene {0} from file: {1}", m_Scene->GetSceneName(), filepath);
					return false;
				}
				return true;
			}
			else
			{
				LOG_BE_ERROR("Failed to deserialize scene {0} from file: {1}", m_Scene->GetSceneName(), filepath);
				return false;
			}
		}
	}
}


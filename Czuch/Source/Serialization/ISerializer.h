#pragma once
#include"yaml-cpp/yaml.h"

namespace Czuch
{
	class SerializerHelper
	{
	public:
		static void Key(const std::string& key)
		{
			(*m_CurrentEmitter) << YAML::Key << key;
		}

		static void Value(const std::string& value)
		{
			(*m_CurrentEmitter) << YAML::Value << value;
		}

		static void KeyVal(const std::string& key, const std::string& value)
		{
			(*m_CurrentEmitter) << YAML::Key << key << YAML::Value << value;
		}

		static void BeginMap()
		{
			(*m_CurrentEmitter) << YAML::BeginMap;
		}

		static void EndMap()
		{
			(*m_CurrentEmitter) << YAML::EndMap;
		}

		static void BeginSeq()
		{
			(*m_CurrentEmitter) << YAML::BeginSeq;
		}

		static void EndSeq()
		{
			(*m_CurrentEmitter) << YAML::EndSeq;
		}
		static void SetEmitter(YAML::Emitter* out)
		{
			m_CurrentEmitter = out;
		}



		static YAML::Emitter* m_CurrentEmitter;
	};



	class ISerializer
	{
	public:
		virtual bool Serialize(YAML::Emitter& out, bool binary = false) = 0;
		virtual bool Deserialize(const YAML::Node& in, bool binary = false) = 0;
	};

	class ISerializationControl
	{
	public:
		virtual bool Serialize(const std::string& filepath, bool binary = false) = 0;
		virtual bool Deserialize(const std::string& filepath, bool binary = false) = 0;
	};
}



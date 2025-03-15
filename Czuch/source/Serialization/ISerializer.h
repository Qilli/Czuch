#pragma once
#include"Core/GUID.h"
#define YAML_CPP_STATIC_DEFINE
#include"yaml-cpp/yaml.h"
#include"Core/Math.h"

namespace YAML
{
	template<>
	struct convert<Vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};	

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};
}


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

		static void ValueGUID(GUID guid)
		{
			(*m_CurrentEmitter) << YAML::Value << (uint64_t)guid;
		}

		static void KeyVal(const std::string& key, const std::string& value)
		{
			(*m_CurrentEmitter) << YAML::Key << key << YAML::Value << value;
		}

		static void KeyGUIDValue(const std::string& key, GUID value)
		{
			(*m_CurrentEmitter) << YAML::Key << key << YAML::Value << (uint64_t)value;
		}

		static void Vec3Val(const Vec3& value)
		{
			(*m_CurrentEmitter) << YAML::Key << YAML::Flow;
			(*m_CurrentEmitter) << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
		}

		static void UIntVal(const uint32_t value)
		{
			(*m_CurrentEmitter) << YAML::Value << value;
		}

		static void IntVal(const int32_t value)
		{
			(*m_CurrentEmitter) << YAML::Value << value;
		}

		static void Vec2Val(const Vec2& value)
		{
			(*m_CurrentEmitter) << YAML::Key << YAML::Flow;
			(*m_CurrentEmitter) << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
		}

		static void Vec4Val(const Vec4& value)
		{
			(*m_CurrentEmitter) << YAML::Key << YAML::Flow;
			(*m_CurrentEmitter) << YAML::BeginSeq << value.x << value.y << value.z << value.w << YAML::EndSeq;
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
		virtual bool Serialize(std::string& filepath, bool binary = false) = 0;
		virtual bool Deserialize(std::string& filepath, bool binary = false) = 0;
	};
}



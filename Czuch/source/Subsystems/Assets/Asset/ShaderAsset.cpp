#include "czpch.h"
#include "ShaderAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"
#include "glslang/Public/ShaderLang.h"
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace Czuch
{

	CzuchStr removeUnnecessaryLines(const CzuchStr& str) {
		std::istringstream iss(str);
		std::ostringstream oss;
		CzuchStr line;

		while (std::getline(iss, line)) {
			if (line != "#extension GL_GOOGLE_include_directive : require" &&
				line.substr(0, 5) != "#line") {
				oss << line << '\n';
			}
		}
		return oss.str();
	}


	class CustomIncluder final : public glslang::TShader::Includer {
	public:
		explicit CustomIncluder(const std::string& shaderDir) : shaderDirectory(shaderDir) {}
		~CustomIncluder() = default;

		IncludeResult* includeSystem(const char* headerName, const char* includerName,
			size_t inclusionDepth) override {
			// You can implement system include paths here if needed.
			return nullptr;
		}

		IncludeResult* includeLocal(const char* headerName, const char* includerName,
			size_t inclusionDepth) override {
			CzuchStr fullPath = shaderDirectory + "/" + headerName;
			std::ifstream fileStream(fullPath, std::ios::in);
			if (!fileStream.is_open()) {
				CzuchStr errMsg = "Failed to open included file: ";
				errMsg.append(headerName);
				return nullptr;
			}

			std::stringstream fileContent;
			fileContent << fileStream.rdbuf();
			fileStream.close();

			// The Includer owns the content memory and will delete it when it is no
			// longer needed.
			char* content = new char[fileContent.str().length() + 1];
			strncpy(content, fileContent.str().c_str(), fileContent.str().length());
			content[fileContent.str().length()] = '\0';

			return new IncludeResult(headerName, content, fileContent.str().length(), nullptr);
		}

		void releaseInclude(IncludeResult* result) override {
			if (result) {
				delete result;
			}
		}

	private:
		CzuchStr shaderDirectory;
	};

	CzuchStr ShaderCodeToString(const Array<char>& code)
	{
		CzuchStr str;
		str.resize(code.size());
		std::memcpy(str.data(), code.data(), code.size());
		return str;
	}

	Array<char> glslToSpirv(const Array<char>& data,
		EShLanguage shaderStage,
		const std::string& shaderDir,
		const char* entryPoint) {
		static bool glslangInitialized = false;

		if (!glslangInitialized) {
			glslang::InitializeProcess();
			glslangInitialized = true;
		}

		glslang::TShader tshadertemp(shaderStage);
		const char* glslCStr = data.data();
		tshadertemp.setStrings(&glslCStr, 1);

		glslang::EshTargetClientVersion clientVersion = glslang::EShTargetVulkan_1_3;
		glslang::EShTargetLanguageVersion langVersion = glslang::EShTargetSpv_1_0;

		if (shaderStage == EShLangRayGen || shaderStage == EShLangAnyHit ||
			shaderStage == EShLangClosestHit || shaderStage == EShLangMiss) {
			langVersion = glslang::EShTargetSpv_1_4;
		}

		tshadertemp.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan,
			460);

		tshadertemp.setEnvClient(glslang::EShClientVulkan, clientVersion);
		tshadertemp.setEnvTarget(glslang::EShTargetSpv, langVersion);

		tshadertemp.setEntryPoint(entryPoint);
		tshadertemp.setSourceEntryPoint(entryPoint);

		glslang::TShader tshader(shaderStage);

		tshader.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan, 460);

		tshader.setEnvClient(glslang::EShClientVulkan, clientVersion);
		tshader.setEnvTarget(glslang::EShTargetSpv, langVersion);

		tshader.setEntryPoint(entryPoint);
		tshader.setSourceEntryPoint(entryPoint);

		const TBuiltInResource* resources = GetDefaultResources();
		const EShMessages messages = static_cast<EShMessages>(
			EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo);
		CustomIncluder includer(shaderDir);

		CzuchStr preprocessedGLSL;
		if (!tshadertemp.preprocess(resources, 460, ENoProfile, false, false, messages,
			&preprocessedGLSL, includer)) {
			LOG_BE_ERROR("Preprocessing failed for shader:\n{0}\n{1}\n{2}", ShaderCodeToString(data), tshadertemp.getInfoLog(), tshadertemp.getInfoDebugLog());
			CZUCH_BE_ASSERT(false, "Error occured");
			return std::vector<char>();
		}

		// required since without this renderdoc have difficulty debugging/stepping
		// through shader correctly
		preprocessedGLSL = removeUnnecessaryLines(preprocessedGLSL);

		const char* preprocessedGLSLStr = preprocessedGLSL.c_str();
		tshader.setStrings(&preprocessedGLSLStr, 1);

		if (!tshader.parse(resources, 460, false, messages)) {
			LOG_BE_ERROR("Parsing failed for shader:\n{0}\n{1}\n{2}",ShaderCodeToString(data),tshader.getInfoLog(), tshader.getInfoDebugLog());
			CZUCH_BE_ASSERT(false, "parse failed");
			return std::vector<char>();
		}

		glslang::SpvOptions options;

#ifdef _DEBUG
		tshader.setDebugInfo(true);
		options.generateDebugInfo = true;
		options.disableOptimizer = true;
		options.optimizeSize = false;
		options.stripDebugInfo = false;
#else
		options.disableOptimizer = true;  // this ensure that variables that aren't
		// used in shaders are not removed, without this flag, SPIRV
		// generated will be optimized & unused variables will be removed,
		// this will cause issues in debug vs release if struct on cpu vs
		// gpu are different
		options.optimizeSize = true;
		options.stripDebugInfo = true;
#endif

		glslang::TProgram program;
		program.addShader(&tshader);
		if (!program.link(messages)) {
			LOG_BE_ERROR("Parsing failed for shader:\n{0}\n{1}", program.getInfoLog(), program.getInfoDebugLog());
			CZUCH_BE_ASSERT(false, "link failed");
		}

		std::vector<uint32_t> spirvData;
		spv::SpvBuildLogger spvLogger;
		glslang::GlslangToSpv(*program.getIntermediate(shaderStage), spirvData, &spvLogger,
			&options);

		std::vector<char> byteCode;
		byteCode.resize(spirvData.size() * (sizeof(uint32_t) / sizeof(char)));
		std::memcpy(byteCode.data(), spirvData.data(), byteCode.size());

		return byteCode;
	}

	ShaderAsset::ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderLoadSettings& loadSettings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path), assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::LOADED_TYPE;
		m_ShaderLoadSettings = std::move(loadSettings);
	}

	ShaderAsset::ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderCreateSettings& settings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path), assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::CREATED_TYPE;
		m_ShaderCreateSettings = std::move(settings);
		CreateFromData();
	}

	ShaderAsset::~ShaderAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	EShLanguage ShaderStageToGlslang(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::VS:
			return EShLangVertex;
		case ShaderStage::PS:
			return EShLangFragment;
		case ShaderStage::GS:
			return EShLangGeometry;
		case ShaderStage::CS:
			return EShLangCompute;
		default:
			return EShLangVertex;
		}
	}

	bool ShaderAsset::LoadAsset()
	{
		if (Asset::LoadAsset())
		{
			return true;
		}

		ShaderStage stage = StringToShaderStage(GetTypeFromPath(AssetPath()));

		if (!ReadTextFile(AssetPath(), m_ShaderCode))
		{
			LOG_BE_ERROR("{0} Failed to load shader resource at path {1}", "[ShaderResource]", AssetPath());
			return false;
		}

		auto glslangStage = ShaderStageToGlslang(stage);

		m_SpirvCode = std::move(glslToSpirv(m_ShaderCode, glslangStage,"Include/","main"));

		if (m_SpirvCode.size() == 0)
		{
			return false;
		}

		m_ShaderAsset = m_Device->CreateShader(stage,m_SpirvCode.data(), m_SpirvCode.size());

		if (!HANDLE_IS_VALID(m_ShaderAsset))
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("Loaded new shader with path: {0}", AssetPath());
		}
		m_RefCounter.Up();
		m_State = AssetInnerState::LOADED;
		return true;
	}

	bool ShaderAsset::UnloadAsset()
	{
		if (ShouldUnload() && HANDLE_IS_VALID(m_ShaderAsset))
		{
			m_Device->Release(m_ShaderAsset);
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}
	bool ShaderAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		ShaderStage stage = m_ShaderCreateSettings.stage;

		if (m_ShaderCreateSettings.shaderCode.size() <= 0)
		{
			LOG_BE_ERROR("{0} Failed to create shader resource from data due to lack of shader code {1}", "[ShaderResource]", AssetPath());
			return false;
		}

		m_ShaderAsset = m_Device->CreateShader(stage, m_ShaderCreateSettings.shaderCode.data(), m_ShaderCreateSettings.shaderCode.size());

		if (!HANDLE_IS_VALID(m_ShaderAsset))
		{
			return false;
		}

		m_State = AssetInnerState::LOADED;
		m_RefCounter.Up();
		return true;
	}
	CzuchStr ShaderAsset::GetAssetLoadInfo() const
	{
		return "ShaderAsset: " + AssetName() + " Ref count: " + m_RefCounter.GetCountString() + " State: " + (m_State == AssetInnerState::LOADED ? " Loaded" : "Unloaded");
	}

	ShortAssetInfo* ShaderAsset::GetShortAssetInfo()
	{
		if (m_ShortInfo.name == nullptr || m_ShortInfo.name->empty())
		{
			m_ShortInfo.name = &AssetName();
			m_ShortInfo.type = AssetType::SHADER;
			m_ShortInfo.asset = GetHandle();
		}

		m_ShortInfo.resource = m_ShaderAsset.handle;

		return &m_ShortInfo;
	}
}

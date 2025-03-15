#pragma once
#include <fstream>
#include<vector>

namespace Czuch
{

    static Array<char> ReadBinaryFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            return Array<char>();
        }

        size_t fileSize = (size_t)file.tellg();
        Array<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    static Array<char> ReadTextFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate);

        if (!file.is_open()) {
            return Array<char>();
        }

		size_t fileSize = (size_t)file.tellg();
		Array<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
    }

    static bool ReadTextFile(const std::string& filename, Array<char>& targetBuffer) {
        std::ifstream file(filename, std::ios::ate);

        if (!file.is_open()) {
            return false;
        }

        size_t fileSize = (size_t)file.tellg();
        targetBuffer.resize(fileSize);

        file.seekg(0);
        file.read(targetBuffer.data(), fileSize);

        file.close();
        return true;
    }



    static bool ReadBinaryFile(const std::string& filename,Array<char>& targetBuffer) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            return false;
        }

        size_t fileSize = (size_t)file.tellg();
        targetBuffer.resize(fileSize);

        file.seekg(0);
        file.read(targetBuffer.data(), fileSize);

        file.close();
        return true;
    }
}
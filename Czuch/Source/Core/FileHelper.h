#pragma once
#include <fstream>
#include<vector>

namespace Czuch
{

    static std::vector<char> ReadBinaryFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            return std::vector<char>();
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }


    static bool ReadBinaryFile(const std::string& filename,std::vector<char>& targetBuffer) {
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
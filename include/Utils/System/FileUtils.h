#pragma once
#include <string>
#include <algorithm>

namespace Utils
{
    namespace System
    {
        bool isValidTxtFile(const std::string& filename)
        {
            if (filename.length() < 4) return false;

            // Lấy 4 ký tự cuối
            std::string ext = filename.substr(filename.find_last_of(".") + 1);

            // Chuyển về chữ thường để so sánh (Case-insensitive)
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            return (ext == "txt");
        }
    }
}

#pragma once
#include <string>
#include <functional>
#include <SFML/System/String.hpp>

namespace DS // Namespace DataStructures (Optional, cho gọn)
{
    // 1. Gói dữ liệu Input: Chứa tất cả những gì UI có thể gửi xuống
    struct InputArgs
    {
        int iVal1 = 0;          // Tham số 1 (Value / Index / Source)
        int iVal2 = 0;          // Tham số 2 (Index / Destination / Value 2)
        std::string sVal = "";  // Tham số chuỗi (File path / String input)

        // Constructors tiện lợi
        InputArgs() {}
        InputArgs(int v1) : iVal1(v1) {}
        InputArgs(int v1, int v2) : iVal1(v1), iVal2(v2) {}
        InputArgs(std::string s) : sVal(s) {}
    };

    // 2. Các loại Input mà UI hỗ trợ hiển thị
    enum class InputType
    {
        None,           // Không hiện gì, bấm là chạy (VD: Clear, Random)
        Integer,        // Hiện 1 ô nhập số (VD: Insert 5)
        TwoIntegers,    // Hiện 2 ô nhập số (VD: Insert(index, value))
        String          // Hiện 1 ô nhập chữ (VD: Load File)
    };

    // 3. Command Struct: Định nghĩa một nút bấm
    struct Command
    {
        sf::String iconCode;           // code của icon cho Button
        std::string name;              // tên hiển thị trên popover
        InputType inputType;        // Loại Input UI cần vẽ
        std::function<void(InputArgs)> action; // Hàm callback logic

        Command(sf::String code, std::string _name, InputType type, std::function<void(InputArgs)> act)
            : iconCode(code), name(_name), inputType(type), action(act) {}
    };
}

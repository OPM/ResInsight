#pragma once

#include <string>
#include <variant>

typedef std::variant<int, float, double, unsigned char, bool, std::string> RoffScalar;

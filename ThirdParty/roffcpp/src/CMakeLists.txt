set(HEADER_LIST "Token.hpp" "Tokenizer.hpp" "AsciiTokenizer.hpp" "BinaryTokenizer.hpp" "Parser.hpp" "AsciiParser.hpp" "Reader.hpp" "RoffScalar.hpp")
set(SOURCE_LIST "Token.cpp" "Tokenizer.cpp" "AsciiTokenizer.cpp" "BinaryTokenizer.cpp" "Parser.cpp" "AsciiParser.cpp" "Reader.cpp")

add_library(roffcpp ${SOURCE_LIST} ${HEADER_LIST})

if(MSVC)
  target_compile_options(roffcpp PRIVATE /W4 /WX)
else()
  target_compile_options(roffcpp PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_include_directories(roffcpp PUBLIC .)

target_compile_features(roffcpp PUBLIC cxx_std_17)

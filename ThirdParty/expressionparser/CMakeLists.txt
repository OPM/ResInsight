cmake_minimum_required (VERSION 2.8)

project (expressionparser)

if(MSVC) 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj") 
    message(STATUS "MSVC: Enabled increased number of sections in object files") 
 endif() 

list (APPEND MAIN_SOURCE_FILES
    ExpressionParser.h
    ExpressionParser.cpp
    ExpressionParserImpl.h
    ExpressionParserImpl.cpp
)

add_library( ${PROJECT_NAME}
    STATIC
    ${MAIN_SOURCE_FILES}
)

if(MSVC) 
    set_target_properties(expressionparser PROPERTIES COMPILE_FLAGS "/wd4267")
 endif() 

 target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})

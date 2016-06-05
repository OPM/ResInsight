cmake_minimum_required (VERSION 2.8)

project (custom-opmjson)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

list (APPEND MAIN_SOURCE_FILES
    opm/json/JsonObject.cpp
    opm/json/cjson/cJSON.c
)

add_library( ${PROJECT_NAME}
    STATIC
    ${MAIN_SOURCE_FILES}
)


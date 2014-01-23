cmake_minimum_required(VERSION 2.8)

project(LibUtilities)


# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    # Don't allow the compiler to assume strict aliasing when doing optimizations (the culprit is JPEG code)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fno-strict-aliasing")
endif()


include_directories(../LibCore)
include_directories(../LibIo)
include_directories(../LibGeometry)
include_directories(../LibRender)
include_directories(../LibViewing)


set(CEE_SOURCE_FILES
cvfuImageJpeg.cpp
cvfuImageTga.cpp
cvfuInputEvents.cpp
cvfuPartCompoundGenerator.cpp
cvfuPointLight.cpp
cvfuProperty.cpp
cvfuSampleFactory.cpp
cvfuSnippetFactory.cpp
cvfuSnippetPropertyPublisher.cpp
cvfuTestSnippet.cpp
cvfuWavefrontObjImport.cpp
)

add_library(${PROJECT_NAME} ${CEE_SOURCE_FILES})

cmake_minimum_required(VERSION 2.8)

project(LibCore_UnitTests)

# Compile flags should already be setup by caller


include_directories(${LibCore_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibCore)


set(CEE_SOURCE_FILES
cvfArray-Test.cpp
cvfArrayWrapper-Test.cpp
cvfAtomicCounter-Test.cpp
cvfBase-Test.cpp
cvfBase64-Test.cpp
cvfCharArray-Test.cpp
cvfCodeLocation-Test.cpp
cvfCollection-Test.cpp
cvfColor3-Test.cpp
cvfColor4-Test.cpp
cvfDebugTimer-Test.cpp
cvfFlags-Test.cpp
cvfLogEvent-Test.cpp
cvfLogger-Test.cpp
cvfMath-Test.cpp
cvfMatrix3-Test.cpp
cvfMatrix4-Test.cpp
cvfObject-Test.cpp
cvfPlane-Test.cpp
cvfProgramOptions-Test.cpp
cvfPropertySet-Test.cpp
cvfPropertySetCollection-Test.cpp
cvfRect-Test.cpp
cvfQuat-Test.cpp
cvfString-Test.cpp
cvfSystem-Test.cpp
cvfTimer-Test.cpp
cvfTrace-Test.cpp
cvfVariant-Test.cpp
cvfVector2-Test.cpp
cvfVector3-Test.cpp
cvfVector4-Test.cpp
../../ThirdParty/gtest/gtest-all.cpp
LibCore_UnitTests.cpp
)

add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${CEE_PLATFORM_LIBS})


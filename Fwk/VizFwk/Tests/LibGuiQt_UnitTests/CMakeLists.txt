project(LibGuiQt_UnitTests)

# Compile flags should already be setup by caller

find_package(OpenGL)

if (CEE_USE_QT6)
    find_package(Qt6 COMPONENTS REQUIRED Core Gui Widgets OpenGLWidgets)
    set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets )
elseif (CEE_USE_QT5)
    find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets OpenGL)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
else()
    message(FATAL_ERROR "No supported Qt version selected for build")
endif()

include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibIo_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibGuiQt_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibGuiQt LibViewing LibRender LibGeometry LibIo LibCore)


set(CEE_SOURCE_FILES
cvfqtUtils-Test.cpp 
../../ThirdParty/gtest/gtest-all.cpp
LibGuiQt_UnitTests.cpp
)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
    # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${QT_LIBRARIES} ${CEE_PLATFORM_LIBS})


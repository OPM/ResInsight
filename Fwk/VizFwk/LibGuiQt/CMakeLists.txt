project(LibGuiQt)

# CMake Unity Build
if (CVF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# We're getting too much trouble from Qt using strict
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    # Due to usage of OpenMP, disable warning about unknown pragmas
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long  -Wno-unknown-pragmas")
endif()

find_package(OpenGL)


message(STATUS "In LibGuiQt, CEE_USE_QT6=${CEE_USE_QT6}")
message(STATUS "In LibGuiQt, CEE_USE_QT5=${CEE_USE_QT5}")
if (CEE_USE_QT6)
    find_package(Qt6 COMPONENTS REQUIRED Core Gui Widgets OpenGLWidgets)
    set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets )
elseif (CEE_USE_QT5)
    find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets OpenGL)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
else()
    message(FATAL_ERROR "No supported Qt version selected for build")
endif()




set(CEE_HEADER_FILES
cvfqtBasicAboutDialog.h
cvfqtMouseState.h
cvfqtOpenGLWidget.h
cvfqtPerformanceInfoHud.h
cvfqtUtils.h
)

set(CEE_SOURCE_FILES
cvfqtBasicAboutDialog.cpp
cvfqtMouseState.cpp
cvfqtOpenGLWidget.cpp
cvfqtPerformanceInfoHud.cpp
cvfqtUtils.cpp
)

if (CEE_USE_QT5)
    set(CEE_HEADER_FILES ${CEE_HEADER_FILES} cvfqtGLWidget.h)
    set(CEE_SOURCE_FILES ${CEE_SOURCE_FILES} cvfqtGLWidget.cpp)
    set(CEE_HEADER_FILES ${CEE_HEADER_FILES} cvfqtCvfBoundQGLContext_deprecated.h)
    set(CEE_SOURCE_FILES ${CEE_SOURCE_FILES} cvfqtCvfBoundQGLContext_deprecated.cpp)
    set(CEE_HEADER_FILES ${CEE_HEADER_FILES} cvfqtGLWidget_deprecated.h)
    set(CEE_SOURCE_FILES ${CEE_SOURCE_FILES} cvfqtGLWidget_deprecated.cpp)
endif()

add_library(${PROJECT_NAME} ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    LibCore
    LibGeometry
    LibRender
    LibViewing

    ${QT_LIBRARIES}
)
set(PROJECT_FILES ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})
source_group("" FILES ${PROJECT_FILES})

# Unity Build
if (CMAKE_UNITY_BUILD)
  set_source_files_properties (cvfqtGLWidget_deprecated.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
endif()

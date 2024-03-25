project(QtMinimal_GLWidget)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STANDARD_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long")
endif()


find_package(OpenGL)

if (CEE_USE_QT5)
    find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL)
    set(QT_LIBRARIES Qt5::Widgets Qt5::OpenGL)
else()
    message(FATAL_ERROR "No supported Qt version selected for build")
endif()


include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibGuiQt_SOURCE_DIR})

set(CEE_LIBS LibGuiQt LibViewing LibRender LibGeometry LibCore)
 

set(CEE_SOURCE_FILES
QMMain_GLW.cpp
QMMainWindow_GLW.cpp
QMWidget_GLW.cpp
)

# Headers that need MOCing
set(MOC_HEADER_FILES
QMMainWindow_GLW.h
QMWidget_GLW.h
)

if (CEE_USE_QT5)
    qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
endif()

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES} ${MOC_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${QT_LIBRARIES})

project(QtTestBenchOpenGLWidget)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STANDARD_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long")
endif()


find_package(OpenGL)

if (CEE_USE_QT6)
    find_package(Qt6 COMPONENTS REQUIRED OpenGLWidgets)
    set(QT_LIBRARIES Qt6::OpenGLWidgets )
elseif (CEE_USE_QT5)
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    set(QT_LIBRARIES Qt5::Widgets)
else()
    message(FATAL_ERROR "No supported Qt version selected for build")
endif()


include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibUtilities_SOURCE_DIR})
include_directories(${LibGuiQt_SOURCE_DIR})

set(CEE_LIBS LibGuiQt LibUtilities LibViewing LibRender LibGeometry LibIo LibCore)


set(CEE_CODE_FILES
QTBMain.cpp
QTBMainWindow.cpp
QTBMainWindow.h
QTBSceneFactory.cpp
QTBSceneFactory.h
QTBVizWidget.cpp
QTBVizWidget.h
)

# Headers that need MOCing
set(MOC_HEADER_FILES
QTBMainWindow.h
QTBVizWidget.h
)

if (CEE_USE_QT6)
    qt_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
elseif (CEE_USE_QT5)
    qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
endif()

add_executable(${PROJECT_NAME} ${CEE_CODE_FILES} ${MOC_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${QT_LIBRARIES})


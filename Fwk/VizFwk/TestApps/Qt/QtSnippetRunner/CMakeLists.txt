project(QtSnippetRunner)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long")
endif()


find_package(OpenGL)

include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibGuiQt_SOURCE_DIR})
include_directories(${LibUtilities_SOURCE_DIR})
include_directories(${SnippetsBasis_SOURCE_DIR})

set(CEE_LIBS SnippetsBasis freetype LibFreeType LibUtilities LibGuiQt LibViewing LibRender LibGeometry LibIo LibCore)

include_directories(${SnippetsModules_SOURCE_DIR})
set(CEE_LIBS LibStructGrid ${CEE_LIBS})


set(CEE_SOURCE_FILES
QSRCommandLineArgs.cpp
QSRMain.cpp
QSRMainWindow.cpp
QSRPropertiesPanel.cpp
QSRRunPanel.cpp
QSRSnippetWidget.cpp
QSRStdInclude.cpp
QSRTranslateEvent.cpp
)

# Headers that need MOCing
set(MOC_HEADER_FILES
QSRMainWindow.h
QSRPropertiesPanel.h
QSRRunPanel.h
QSRSnippetWidget.h
)

# Qt
if (CEE_USE_QT5)
    find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets OpenGL)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
    qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
else()
    message(FATAL_ERROR "No supported Qt version selected for build")
endif()

set(SYSTEM_LIBRARIES)
if (CMAKE_COMPILER_IS_GNUCXX)
	set(SYSTEM_LIBRARIES -lrt -lpthread)
endif(CMAKE_COMPILER_IS_GNUCXX)

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES} ${MOC_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${QT_LIBRARIES} ${SYSTEM_LIBRARIES})

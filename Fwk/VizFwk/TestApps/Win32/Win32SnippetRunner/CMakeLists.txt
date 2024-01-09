project(Win32SnippetRunner)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STANDARD_CXX_FLAGS}")


find_package(OpenGL)


include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibUtilities_SOURCE_DIR})
include_directories(${SnippetsBasis_SOURCE_DIR})

set(CEE_LIBS SnippetsBasis freetype LibFreeType LibUtilities LibViewing LibRender LibGeometry LibIo LibCore)

set(CEE_CODE_FILES
Win32MessageKicker.cpp
Win32MessageKicker.h
Win32OpenGLContext.cpp
Win32OpenGLContext.h
Win32PropertiesPanel.cpp
Win32PropertiesPanel.h
Win32SnippetRunner.cpp
Win32SnippetWindow.cpp
Win32SnippetWindow.h
Win32Utils.cpp
Win32Utils.h
)


add_executable(${PROJECT_NAME} WIN32 ${CEE_CODE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES})


cmake_minimum_required(VERSION 2.8.12)

project(cafUserInterface_UnitTests)

find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets
)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_FILES cafUserInterface_UnitTests.cpp cafPdmUiTreeViewModelTest.cpp
                  cafPdmUiTreeSelectionQModelTest.cpp gtest/gtest-all.cpp
)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

source_group("" FILES ${PROJECT_FILES})

target_link_libraries(
  ${PROJECT_NAME} cafUserInterface ${QT_LIBRARIES} ${THREAD_LIBRARY}
)

# Copy Qt Dlls
foreach(qtlib ${QT_LIBRARIES})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
  )
endforeach(qtlib)

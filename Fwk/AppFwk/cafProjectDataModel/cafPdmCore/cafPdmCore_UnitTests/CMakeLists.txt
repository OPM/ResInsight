cmake_minimum_required(VERSION 2.8.12)

project(cafPdmCore_UnitTests)

# Qt
find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Gui)

if(MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code
  # related to TR1
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message(
    STATUS
      "Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING"
  )
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

set(PROJECT_FILES
    cafPdmCore_UnitTests.cpp
    gtest/gtest-all.cpp
    cafPdmCoreBasicTest.cpp
    cafPdmReferenceHelperTest.cpp
    cafPdmChildArrayFieldHandleTest.cpp
    cafSignalTest.cpp
    Child.cpp
    Child.h
    Parent.cpp
    Parent.h
    TestObj.cpp
    TestObj.h)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

source_group("" FILES ${PROJECT_FILES})

if(Qt5Core_FOUND)
  set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

target_link_libraries(${PROJECT_NAME} cafPdmCore ${QT_LIBRARIES}
                      ${THREAD_LIBRARY})

# Copy Qt Dlls
foreach(qtlib ${QT_LIBRARIES})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach(qtlib)

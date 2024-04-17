project (expressionparser)

if(MSVC) 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj") 
    message(STATUS "MSVC: Enabled increased number of sections in object files") 
 endif() 

if (CEE_USE_QT6)
    find_package(Qt6 COMPONENTS REQUIRED Core)
    set(QT_LIBRARIES Qt6::Core)
    qt_standard_project_setup()
else()
    find_package(Qt5 COMPONENTS REQUIRED Core)
    set(QT_LIBRARIES Qt5::Core)
endif()

list (APPEND MAIN_SOURCE_FILES
    ExpressionParser.h
    ExpressionParser.cpp
    ExpressionParserImpl.h
    ExpressionParserImpl.cpp
)

add_library( ${PROJECT_NAME}
    STATIC
    ${MAIN_SOURCE_FILES}
)

if(MSVC) 
    set_target_properties(expressionparser PROPERTIES COMPILE_FLAGS "/wd4267")
 endif() 

 target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})

cmake_minimum_required (VERSION 2.8.12)

project (cafCommand)

# Unity Build
if (COMMAND caf_apply_unity_build)
  caf_apply_unity_build("${PROJECT_NAME}")
endif()

# These headers need to go through Qt's MOC compiler
set (MOC_HEADER_FILES    
    cafCmdFeature.h
	cafCmdFeatureManager.h
)

# Qt
if (CEE_USE_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
else()
    find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
    include(${QT_USE_FILE})
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
endif(CEE_USE_QT5)

set( PROJECT_FILES

    cafCmdExecCommandManager.cpp
    cafCmdExecCommandManager.h
    cafCmdExecuteCommand.h
    cafCmdUiCommandSystemImpl.h
    cafCmdUiCommandSystemImpl.cpp

    cafCmdFieldChangeExec.cpp
    cafCmdFieldChangeExec.h

    cafCmdSelectionHelper.cpp
    cafCmdSelectionHelper.h
    cafCmdSelectionChangeExec.cpp
    cafCmdSelectionChangeExec.h

    cafCmdFeature.cpp
    cafCmdFeature.h
    cafCmdFeatureManager.cpp
    cafCmdFeatureManager.h
	cafCmdFeatureMenuBuilder.cpp
	cafCmdFeatureMenuBuilder.h
)


add_library( ${PROJECT_NAME}
 	${PROJECT_FILES}
    ${MOC_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
	${QT_LIBRARIES}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()

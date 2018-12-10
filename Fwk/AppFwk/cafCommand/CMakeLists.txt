cmake_minimum_required (VERSION 2.8.12)

# Qt
if (RESINSIGHT_BUILD_WITH_QT5)
	find_package(Qt5 COMPONENTS Core QUIET)
endif(RESINSIGHT_BUILD_WITH_QT5)

if (Qt5Core_FOUND)
	find_package(Qt5 CONFIG REQUIRED Core Gui Widgets)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
    include(${QT_USE_FILE})
endif(Qt5Core_FOUND)

project (cafCommand)


# These headers need to go through Qt's MOC compiler
set (MOC_HEADER_FILES    
    cafCmdFeature.h
	cafCmdFeatureManager.h
)

if (Qt5Core_FOUND)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
else()
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
endif()

set( PROJECT_FILES

    cafCmdExecCommandManager.cpp
    cafCmdExecCommandManager.h
    cafCmdExecuteCommand.h
    cafCmdUiCommandSystemImpl.h
    cafCmdUiCommandSystemImpl.cpp

    # Default features
    defaultfeatures/cafCmdAddItemExec.cpp
    defaultfeatures/cafCmdAddItemExec.h
    defaultfeatures/cafCmdAddItemExecData.cpp
    defaultfeatures/cafCmdAddItemExecData.h
    defaultfeatures/cafCmdAddItemFeature.cpp
    defaultfeatures/cafCmdAddItemFeature.h
    defaultfeatures/cafCmdDeleteItemExec.cpp
    defaultfeatures/cafCmdDeleteItemExec.h
    defaultfeatures/cafCmdDeleteItemExecData.cpp
    defaultfeatures/cafCmdDeleteItemExecData.h
    defaultfeatures/cafCmdDeleteItemFeature.cpp
    defaultfeatures/cafCmdDeleteItemFeature.h

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

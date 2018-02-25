cmake_minimum_required (VERSION 2.8)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
include (${QT_USE_FILE})

project (cafCommand)


# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafCmdFeature.h
	cafCmdFeatureManager.h
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
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
    ${MOC_FILES_CPP}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
)


source_group("" FILES ${PROJECT_FILES})
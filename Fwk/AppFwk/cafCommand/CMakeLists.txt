cmake_minimum_required (VERSION 2.8)

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui)
else()
  find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
  include (${QT_USE_FILE})
endif()

project (cafCommand)

include_directories(
    ${cafProjectDataModel_SOURCE_DIR}
)

include_directories (
    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
    .
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafCmdFeature.h
	cafCmdFeatureManager.h
)

if(USE_QT5)
  qt5_wrap_cpp(MOC_FILES_CPP ${QOBJECT_HEADERS})
else()
  if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
  endif()
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
)


add_library( ${PROJECT_NAME}
 	${PROJECT_FILES}
    ${MOC_FILES_CPP}
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui)
endif()
source_group("" FILES ${PROJECT_FILES})

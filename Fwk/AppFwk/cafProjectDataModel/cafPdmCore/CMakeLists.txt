cmake_minimum_required (VERSION 2.8.12)

project (cafPdmCore)

# Qt
if (RESINSIGHT_BUILD_WITH_QT5)
	find_package(Qt5 COMPONENTS Core QUIET)
endif(RESINSIGHT_BUILD_WITH_QT5)

if (Qt5Core_FOUND)
	find_package(Qt5 CONFIG REQUIRED Core)
else()
	find_package(Qt4 COMPONENTS QtCore REQUIRED)
    include(${QT_USE_FILE})
endif(Qt5Core_FOUND)

set( PROJECT_FILES

    cafAssert.h
    cafAppEnum.h
    cafClassTypeName.h
    cafPdmBase.h
    cafPdmChildArrayField.h
    cafPdmChildArrayField.inl
    cafPdmChildArrayFieldHandle.cpp
    cafPdmChildField.h
    cafPdmChildField.inl
    cafPdmDataValueField.h
    cafPdmFieldCapability.h
    cafPdmFieldHandle.cpp
    cafPdmFieldHandle.h
    cafPdmObjectCapability.h
    cafPdmObjectHandle.cpp
    cafPdmObjectHandle.h
    cafPdmPointer.cpp
    cafPdmPointer.h
    cafPdmProxyValueField.h
    cafPdmPtrArrayField.h
    cafPdmPtrArrayField.inl
    cafPdmPtrArrayFieldHandle.h
    cafPdmPtrField.h
    cafPdmPtrField.inl
    cafPdmReferenceHelper.cpp
    cafPdmReferenceHelper.h
    cafPdmUiFieldHandleInterface.h
    cafPdmValueField.h
    cafInternalPdmValueFieldSpecializations.h 

    cafNotificationCenter.cpp
    cafNotificationCenter.h

    cafTristate.cpp
    cafTristate.h
    cafFilePath.cpp
    cafFilePath.h
    cafAsyncObjectDeleter.h
    cafAsyncObjectDeleter.inl
	cafAsyncWorkerManager.h
	cafAsyncWorkerManager.cpp
)

add_library( ${PROJECT_NAME}
    ${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (Qt5Core_FOUND)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})

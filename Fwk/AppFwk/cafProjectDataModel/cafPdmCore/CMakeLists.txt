cmake_minimum_required (VERSION 2.8)

project (cafPdmCore)

# Qt
find_package ( Qt4 COMPONENTS QtCore )
include (${QT_USE_FILE})

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
)

add_library( ${PROJECT_NAME}
    ${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})

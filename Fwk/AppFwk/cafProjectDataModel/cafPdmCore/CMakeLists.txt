cmake_minimum_required (VERSION 3.12)

project (cafPdmCore)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

find_package(Qt5 COMPONENTS REQUIRED Core)
set(QT_LIBRARIES Qt5::Core)

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
    cafPdmField.h
    cafPdmFieldCapability.h
    cafPdmFieldHandle.cpp
    cafPdmFieldHandle.h
    cafPdmObjectCapability.h
    cafPdmFieldReorderCapability.cpp
    cafPdmFieldReorderCapability.h
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

    cafSignal.h
    cafSignal.cpp

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

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})

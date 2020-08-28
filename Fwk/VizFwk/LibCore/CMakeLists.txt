cmake_minimum_required(VERSION 2.8)

project(LibCore)

# CMake Unity Build
if (CVF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations")
endif()

set(CEE_HEADER_FILES
cvfArray.h
cvfArray.inl
cvfArrayWrapperConst.h
cvfArrayWrapperToEdit.h
cvfAssert.h
cvfAtomicCounter.h
cvfBase.h
cvfBase64.h
cvfCharArray.h
cvfCodeLocation.h
cvfCollection.h
cvfCollection.inl
cvfColor3.h
cvfColor4.h
cvfConfigCore.h
cvfDebugTimer.h
cvfFlags.h
cvfFlags.inl
cvfFunctorRange.h
cvfLibCore.h
cvfLogDestination.h
cvfLogDestinationConsole.h
cvfLogDestinationFile.h
cvfLogEvent.h
cvfLogManager.h
cvfLogger.h
cvfMath.h
cvfMath.inl
cvfMatrix3.h
cvfMatrix3.inl
cvfMatrix4.h
cvfMatrix4.inl
cvfMutex.h
cvfObject.h
cvfObject.inl
cvfPlane.h
cvfProgramOptions.h
cvfPropertySet.h
cvfPropertySetCollection.h
cvfQuat.h
cvfQuat.inl
cvfRect.h
cvfRect.inl
cvfString.h
cvfSystem.h
cvfTBBControl.h
cvfTimer.h
cvfTrace.h
cvfValueArray.h
cvfVariant.h
cvfVector2.h
cvfVector2.inl
cvfVector3.h
cvfVector3.inl
cvfVector4.h
cvfVector4.inl
cvfVersion.h
)

set(CEE_SOURCE_FILES
cvfAssert.cpp 
cvfAtomicCounter.cpp
cvfBase64.cpp
cvfCharArray.cpp
cvfCodeLocation.cpp
cvfColor3.cpp
cvfColor4.cpp
cvfDebugTimer.cpp
cvfLogDestinationConsole.cpp
cvfLogDestinationFile.cpp
cvfLogEvent.cpp
cvfLogManager.cpp
cvfLogger.cpp
cvfMath.cpp
cvfMutex.cpp
cvfObject.cpp
cvfPlane.cpp
cvfProgramOptions.cpp
cvfPropertySet.cpp
cvfPropertySetCollection.cpp
cvfString.cpp
cvfSystem.cpp
cvfTimer.cpp
cvfTBBControl.cpp
cvfTrace.cpp
cvfVariant.cpp
cvfVector2.cpp
cvfVector3.cpp
cvfVector4.cpp
)

add_library(${PROJECT_NAME} ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PROJECT_FILES ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})
source_group("" FILES ${PROJECT_FILES})

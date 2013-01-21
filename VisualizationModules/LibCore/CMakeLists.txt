cmake_minimum_required(VERSION 2.8)

project(LibCore)


# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")


set(CEE_HEADER_FILES
cvfArray.h
cvfArray.inl
cvfAssert.h
cvfBase.h
cvfBase64.h
cvfCharArray.h
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
cvfLogger.h
cvfMath.h
cvfMath.inl
cvfMatrix3.h
cvfMatrix3.inl
cvfMatrix4.h
cvfMatrix4.inl
cvfObject.h
cvfObject.inl
cvfPlane.h
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
cvfBase64.cpp
cvfCharArray.cpp
cvfColor3.cpp
cvfColor4.cpp
cvfDebugTimer.cpp
cvfLogger.cpp
cvfMath.cpp
cvfObject.cpp
cvfPlane.cpp
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


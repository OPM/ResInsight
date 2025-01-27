project(RifGeoMechFileInterface)

# Unity Build
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

add_library(
  ${PROJECT_NAME}
  RifInpReader.h RifInpReader.cpp RifGeoMechReaderInterface.h
  RifGeoMechReaderInterface.cpp RifInpIncludeReader.h RifInpIncludeReader.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(LINK_LIBRARIES RigGeoMechDataModel LibCore)

target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})

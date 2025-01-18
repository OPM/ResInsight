cmake_minimum_required(VERSION 3.15)
project(example_project)

# Find pybind11
find_package(pybind11 REQUIRED)

# Add the module
pybind11_add_module(msjmodule example.cpp)

set(LINK_LIBRARIES
    ${THIRD_PARTY_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPM_LIBRARIES}
    ${APP_FWK_LIBRARIES}
    ${VIZ_FWK_LIBRARIES}
    ApplicationLibCode
    Commands
    RigGeoMechDataModel
    RifGeoMechFileInterface
)

#if(RESINSIGHT_ENABLE_GRPC)
#  list(APPEND LINK_LIBRARIES GrpcInterface)
#endif()

if(RESINSIGHT_USE_ODB_API)
  list(APPEND LINK_LIBRARIES RifOdbReader)
endif()

target_link_libraries(
  msjmodule PUBLIC ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES}
)

if(MSVC)
  add_custom_command(
    TARGET msjmodule
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:msjmodule>
      $<TARGET_FILE_DIR:msjmodule>
    COMMAND_EXPAND_LISTS
  )

endif(MSVC)

# ##############################################################################
# Copy required Dll/.so to output folder
# ##############################################################################

# create an empty library target that will be used to copy files to the build
# folder
add_library(ResInsightDummyTestTarget2 EXCLUDE_FROM_ALL empty.cpp)
set_property(
  TARGET ResInsightDummyTestTarget2 PROPERTY FOLDER "FileCopyTargetsTest"
)

# create a custom target that copies the files to the build folder
foreach(riFileName ${RI_FILENAMES})
  list(
    APPEND
    copyCommands
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_if_different
    ${riFileName}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/native-python
  )
endforeach()
add_custom_target(PreBuildFileCopyTest2 ${copyCommands})
set_property(TARGET PreBuildFileCopyTest2 PROPERTY FOLDER "FileCopyTargetsTest")

add_dependencies(msjmodule PreBuildFileCopyTest2)

install(TARGETS msjmodule DESTINATION .)

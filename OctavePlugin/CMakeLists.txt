# mkoctfile -I../../QtInstall/Qt-4.7.3/include/QtNetwork -I../../QtInstall/Qt-4.7.3/include/QtCore -I../../QtInstall/Qt-4.7.3/include -L../../QtInstall/Qt-4.7.3/lib -lQtCore -lQtNetwork getEclipseProperty.cpp

set(CPP_SOURCES
  riGetActiveCellProperty.cpp
  riSetActiveCellProperty.cpp
  riGetActiveCellInfo.cpp
  riGetMainGridDimensions.cpp
  riGetCurrentCase.cpp
  riGetCaseGroups.cpp
  riGetSelectedCases.cpp
  riGetCases.cpp
  riGetTimeStepDates.cpp
  riGetTimeStepDays.cpp
  riGetGridDimensions.cpp
  riGetCoarseningInfo.cpp
  riGetCellCenters.cpp
  riGetActiveCellCenters.cpp
  riGetCellCorners.cpp
  riGetActiveCellCorners.cpp
  riGetGridProperty.cpp
  riSetGridProperty.cpp
  riGetPropertyNames.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # since the compiler passes the option to the linker, double quoting is necessary
  set (RPATH_COMMAND "-Wl,-rpath,'\\$$ORIGIN'")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (QT_LIBRARY_POSTFIX 4)
endif()

set(OCTAVE_BINARY_OCT_FILES)

# On Windows, the Octave plugins are compiled using 32-bit VS2010
# To be able to do so, we need to establish all Qt-related variables for a 32-bit configuration
# In addition, VS2010 32-bit compile environment must be launched
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND CMAKE_CL_64)
    find_program(RESINSIGHT_OCTAVE_PLUGIN_32BIT_QMAKE_EXE 32bitqmake)
    if(RESINSIGHT_OCTAVE_PLUGIN_32BIT_QMAKE_EXE)
        get_filename_component(32BIT_QMAKE_PATH ${RESINSIGHT_OCTAVE_PLUGIN_32BIT_QMAKE_EXE} PATH)    # Get path to 32-bit Qt binary directory
        STRING(REPLACE "/bin" "" OCTAVE_QT_ROOT ${32BIT_QMAKE_PATH})

        SET(OCTAVE_QT_INCLUDE_DIR            ${OCTAVE_QT_ROOT}/include)
        SET(OCTAVE_QT_QTCORE_INCLUDE_DIR     ${OCTAVE_QT_ROOT}/include/QtCore)
        SET(OCTAVE_QT_QTNETWORK_INCLUDE_DIR  ${OCTAVE_QT_ROOT}/include/QtNetwork)
        SET(OCTAVE_QT_LIBRARY_DIR            ${OCTAVE_QT_ROOT}/lib)
    endif()
else()
    SET(OCTAVE_QT_INCLUDE_DIR            ${QT_INCLUDE_DIR})
    SET(OCTAVE_QT_QTCORE_INCLUDE_DIR     ${QT_QTCORE_INCLUDE_DIR})
    SET(OCTAVE_QT_QTNETWORK_INCLUDE_DIR  ${QT_QTNETWORK_INCLUDE_DIR})
    SET(OCTAVE_QT_LIBRARY_DIR            ${QT_LIBRARY_DIR})
endif()

    
find_program(RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE_EXE mkoctfile)
if(NOT RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE_EXE)
  message(WARNING "Failed to find mkoctfile")
else()

  # Get path to Octave binary directory to be able to build .oct files if Octave is not in path
  get_filename_component(OCTAVE_PATH ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE_EXE} PATH)

  foreach(srcFileName IN LISTS CPP_SOURCES)
  
    if(NOT IS_ABSOLUTE "${srcFileName}")
        set(srcFileName "${CMAKE_CURRENT_SOURCE_DIR}/${srcFileName}")
    endif()
  
    get_filename_component(baseFilename "${srcFileName}" NAME_WE)

    set(octFileName "${CMAKE_CURRENT_BINARY_DIR}/${baseFilename}.oct")
    
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        if (CMAKE_CL_64)
            # The following line calls the build configuration for x86 VS2010 compiler
            # call "\"%VS100COMNTOOLS%../../VC/vcvarsall.bat\"" x86
            add_custom_command(
                OUTPUT "${octFileName}"
                COMMAND call "\"%VS100COMNTOOLS%../../VC/vcvarsall.bat\"" x86
                COMMAND ${CMAKE_COMMAND} ARGS -E chdir ${OCTAVE_PATH} ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE_EXE} -I${OCTAVE_QT_QTNETWORK_INCLUDE_DIR}
                    -I${OCTAVE_QT_QTCORE_INCLUDE_DIR} -I${OCTAVE_QT_INCLUDE_DIR} ${RPATH_COMMAND}
                    -L${OCTAVE_QT_LIBRARY_DIR} -lQtCore${QT_LIBRARY_POSTFIX} -lQtNetwork${QT_LIBRARY_POSTFIX} -o "${octFileName}" "${srcFileName}"
                DEPENDS "${srcFileName}"
                COMMENT "Generating ${octFileName}"
            )
        else()
            add_custom_command(
                OUTPUT "${octFileName}"
                COMMAND ${CMAKE_COMMAND} ARGS -E chdir ${OCTAVE_PATH} ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE_EXE} -I${OCTAVE_QT_QTNETWORK_INCLUDE_DIR}
                    -I${OCTAVE_QT_QTCORE_INCLUDE_DIR} -I${OCTAVE_QT_INCLUDE_DIR} ${RPATH_COMMAND}
                    -L${OCTAVE_QT_LIBRARY_DIR} -lQtCore${QT_LIBRARY_POSTFIX} -lQtNetwork${QT_LIBRARY_POSTFIX} -o "${octFileName}" "${srcFileName}"
                DEPENDS "${srcFileName}"
                COMMENT "Generating ${octFileName}"
            )
        endif()
    else()
        add_custom_command(
            OUTPUT "${octFileName}"
            COMMAND ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE_EXE} -I${QT_QTNETWORK_INCLUDE_DIR} -I${QT_QTCORE_INCLUDE_DIR} -I${QT_INCLUDE_DIR} ${RPATH_COMMAND} -L${QT_LIBRARY_DIR} -lQtCore${QT_LIBRARY_POSTFIX} -lQtNetwork${QT_LIBRARY_POSTFIX} -o "${octFileName}" "${srcFileName}"
            DEPENDS "${srcFileName}"
            COMMENT "Generating ${octFileName}"
        )
    endif()

    list(APPEND OCTAVE_BINARY_OCT_FILES "${octFileName}")
  
  endforeach()
  
  add_custom_target(octave_plugins ALL DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellProperty.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riSetActiveCellProperty.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellInfo.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetMainGridDimensions.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCurrentCase.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCaseGroups.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetSelectedCases.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCases.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetTimeStepDates.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetTimeStepDays.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetGridDimensions.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCoarseningInfo.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCellCenters.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellCenters.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCellCorners.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellCorners.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetGridProperty.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riSetGridProperty.oct"
	  "${CMAKE_CURRENT_BINARY_DIR}/riGetPropertyNames.oct"
      SOURCES ${CPP_SOURCES}
  )

    # Copy Octave generated *.oct files to application folder, will make it possible to use Octave functions
    # directly from the location of the ResInsight binaries
    if (MSVC)
        foreach (oct_bin ${OCTAVE_BINARY_OCT_FILES})
            
            get_filename_component(Filename "${oct_bin}" NAME)

            add_custom_command(TARGET octave_plugins POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${oct_bin}"
                "${CMAKE_CURRENT_BINARY_DIR}/../ApplicationCode/$<CONFIGURATION>/${Filename}"
            )
        endforeach( oct_bin )
    endif(MSVC)

    # Make ResInsight dependant on Octave, makes it easiser to debug Octave functionality by compiling ResInsight
    add_dependencies(ResInsight octave_plugins)

endif()

if (RESINSIGHT_PRIVATE_INSTALL)
install(FILES ${OCTAVE_BINARY_OCT_FILES} DESTINATION ${RESINSIGHT_FINAL_NAME})
else (RESINSIGHT_PRIVATE_INSTALL)
  # probe for site location of .oct files
  if (NOT OCTAVE_SITE_OCT_DIR)
	find_program (OCTAVE_CONFIG_COMMAND
	  octave-config
	  DOC "Path to Octave component and library information retrieval"
	  )
	exec_program (${OCTAVE_CONFIG_COMMAND}
	  ARGS --oct-site-dir
	  OUTPUT_VARIABLE OCTAVE_SITE_OCT_DIR
	  )
	set (OCTAVE_SITE_OCT_DIR "${OCTAVE_SITE_OCT_DIR}" CACHE LOCATION "Octave plugin directory")
  endif (NOT OCTAVE_SITE_OCT_DIR)

  install (FILES ${OCTAVE_BINARY_OCT_FILES}
	DESTINATION ${OCTAVE_SITE_OCT_DIR}
	)
endif (RESINSIGHT_PRIVATE_INSTALL)



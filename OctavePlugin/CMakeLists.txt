
# Some of the functionality in this CMakeLists.txt might be easier to maintain if we use a CMake macro to find configuration info for Octave
# This file is currently (2.8.12) not part of default CMake installation
# See http://www.cmake.org/Wiki/CMakeUserFindOctave

set(CPP_SOURCES
    riGetActiveCellProperty.cpp
    riSetActiveCellProperty.cpp
    riGetActiveCellInfo.cpp
    riGetMainGridDimensions.cpp
    riGetNNCConnections.cpp
    riGetCurrentCase.cpp
    riGetCaseGroups.cpp
    riGetDynamicNNCValues.cpp
    riGetStaticNNCValues.cpp
    riGetSelectedCases.cpp
    riGetSelectedCells.cpp
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
    riGetGridPropertyForSelectedCells.cpp
    riGetPropertyNames.cpp
    riGetWellNames.cpp
    riGetWellStatus.cpp
    riGetWellCells.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # since the compiler passes the option to the linker, double quoting is necessary
  set (RPATH_COMMAND "-Wl,-rpath,'\\$$ORIGIN'")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (QT_LIBRARY_POSTFIX 4)

    if (CMAKE_CL_64)
        option(RESINSIGHT_OCTAVE_PLUGIN_32_BIT "Set 32-bit MSVC compiler environment while while running mkoctfile" OFF)
    endif()
endif()

# recreate the magic that CMake does for MacOS X frameworks in the
# include list when we call mkoctfile as a custom command
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (QT_INCLUDES)
    set (QT_FRAMEWORKS)
    # QT_INCLUDE_DIR contains two items; the first is the directory
    # containing header files, the second is the framework. This
    # setup is specially processed in include_directories (); CMake
    # will add -F before the frameworks. We will have to replicate
    # that setup here when we want to pass it directly to a command
    # see <http://www.cmake.org/Bug/print_bug_page.php?bug_id=10632>
    foreach (item IN ITEMS ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_INCLUDE_DIR})
        if ("${item}" MATCHES ".framework$")
            get_filename_component (frmwrk_path ${item} PATH)
            get_filename_component (frmwrk_name ${item} NAME_WE)
            # mkoctfile doesn't support arbitrary compiler command,
            # so we must wrap in -Wl, to pass to the linker
            list (APPEND QT_FRAMEWORKS "-Wl,-F${frmwrk_path}")
            list (APPEND QT_FRAMEWORKS "-Wl,-framework,${frmwrk_name}")
        else ()
            list (APPEND QT_INCLUDES "-I${item}")
        endif ()
    endforeach (item)
    if (QT_INCLUDES)
        list (REMOVE_DUPLICATES QT_INCLUDES)
    endif ()
    if (QT_FRAMEWORKS)
        list (REMOVE_DUPLICATES QT_FRAMEWORKS)
    endif ()
endif ()

# Find location of Octave based on mkoctfile
find_program(RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE mkoctfile)
if(RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE)
    get_filename_component(RESINSIGHT_OCTAVE_BIN_DIR ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE} PATH)
    STRING(REPLACE "/bin" "" OCTAVE_HOME ${RESINSIGHT_OCTAVE_BIN_DIR})
else()
  message(WARNING "Failed to find mkoctfile, no Octave plugins will be compiled. Please specify RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE")
endif()


set(RESINSIGHT_OCTAVE_PLUGIN_QMAKE "" CACHE FILEPATH "Location of Qt to use when compiling Octave plugins. Must be compatible with octave runtime. (Use the Qt embedded in Octave)( The qmake.exe itself is not used, only the path to the directory.)")
if(RESINSIGHT_OCTAVE_PLUGIN_QMAKE)
    get_filename_component(OCTAVE_QMAKE_DIR ${RESINSIGHT_OCTAVE_PLUGIN_QMAKE} PATH)

    STRING(REPLACE "/bin" "" OCTAVE_QT_ROOT ${OCTAVE_QMAKE_DIR})

    message(STATUS "Compiling Octave plugins using custom Qt located at ${OCTAVE_QT_ROOT}")

    SET(OCTAVE_QT_INCLUDE_DIR            ${OCTAVE_QT_ROOT}/include)
    SET(OCTAVE_QT_QTCORE_INCLUDE_DIR     ${OCTAVE_QT_ROOT}/include/QtCore)
    SET(OCTAVE_QT_QTNETWORK_INCLUDE_DIR  ${OCTAVE_QT_ROOT}/include/QtNetwork)
    SET(OCTAVE_QT_LIBRARY_DIR            ${OCTAVE_QT_ROOT}/lib)
else()
    set (RESINSIGHT_OCTAVE_PLUGIN_QMAKE ${QT_QMAKE_EXECUTABLE})

    message(STATUS "Compiling Octave plugins using system Qt - include path located at ${RESINSIGHT_OCTAVE_PLUGIN_QMAKE}")

    SET(OCTAVE_QT_INCLUDE_DIR            ${QT_INCLUDE_DIR})
    SET(OCTAVE_QT_QTCORE_INCLUDE_DIR     ${QT_QTCORE_INCLUDE_DIR})
    SET(OCTAVE_QT_QTNETWORK_INCLUDE_DIR  ${QT_QTNETWORK_INCLUDE_DIR})
    SET(OCTAVE_QT_LIBRARY_DIR            ${QT_LIBRARY_DIR})
endif()


# Clear the list of binary oct files to be produced
set(OCTAVE_BINARY_OCT_FILES)

if (RESINSIGHT_OCTAVE_PLUGIN_QMAKE AND RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE)
    foreach(srcFileName IN LISTS CPP_SOURCES)

        if(NOT IS_ABSOLUTE "${srcFileName}")
            set(srcFileName "${CMAKE_CURRENT_SOURCE_DIR}/${srcFileName}")
        endif()

        get_filename_component(baseFilename "${srcFileName}" NAME_WE)

        set(octFileName "${CMAKE_CURRENT_BINARY_DIR}/${baseFilename}.oct")

        if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
            if (CMAKE_CL_64 AND RESINSIGHT_OCTAVE_PLUGIN_32_BIT)
                SET(RESINSIGHT_VCVARS_32BIT_CMD call "\"%VS100COMNTOOLS%../../VC/vcvarsall.bat\"" x86)
            endif()
            add_custom_command(
                OUTPUT "${octFileName}"
            	COMMAND ${RESINSIGHT_VCVARS_32BIT_CMD}
                COMMAND set "OCTAVE_HOME=${OCTAVE_HOME}"
            	COMMAND set "PATH=%OCTAVE_HOME%\\bin;%PATH%"
                COMMAND mkoctfile -I${OCTAVE_QT_QTNETWORK_INCLUDE_DIR}
            				      -I${OCTAVE_QT_QTCORE_INCLUDE_DIR} 
            				      -I${OCTAVE_QT_INCLUDE_DIR}
            				      -I${ResInsight_SOURCE_DIR}/ApplicationCode/SocketInterface 
            				      ${RPATH_COMMAND} -L${OCTAVE_QT_LIBRARY_DIR} -lQtCore${QT_LIBRARY_POSTFIX} -lQtNetwork${QT_LIBRARY_POSTFIX} 
            					  -o "${octFileName}" "${srcFileName}"
                DEPENDS "${srcFileName}"
                COMMENT "===> Generating ${octFileName}"
            )
        elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            add_custom_command(
                OUTPUT "${octFileName}"
                COMMAND ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE}
                    ${QT_INCLUDES}
                    ${QT_FRAMEWORKS}
                    -I${ResInsight_SOURCE_DIR}/ApplicationCode/SocketInterface
                    ${RPATH_COMMAND}
                    -L${QT_LIBRARY_DIR} -Wl,-framework,QtCore -Wl,-framework,QtNetwork
                    -o "${octFileName}"
                    "${srcFileName}"
                DEPENDS "${srcFileName}"
                COMMENT "===> Generating ${octFileName}"
            )			
        else()
            add_custom_command(
                OUTPUT "${octFileName}"
                COMMAND OCTAVE_HOME=${OCTAVE_HOME} ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE}
                    -I${OCTAVE_QT_QTNETWORK_INCLUDE_DIR} -I${OCTAVE_QT_QTCORE_INCLUDE_DIR} -I${OCTAVE_QT_INCLUDE_DIR} -I${ResInsight_SOURCE_DIR}/ApplicationCode/SocketInterface ${RPATH_COMMAND}
                    -L${OCTAVE_QT_LIBRARY_DIR} -lQtCore -lQtNetwork -o "${octFileName}" "${srcFileName}"
                DEPENDS "${srcFileName}"
                COMMENT "===> Generating ${octFileName}"
            )
        endif()

        list(APPEND OCTAVE_BINARY_OCT_FILES "${octFileName}")

    endforeach()

    add_custom_target(octave_plugins ALL DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellProperty.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riSetActiveCellProperty.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellInfo.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetMainGridDimensions.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetNNCConnections.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetDynamicNNCValues.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetStaticNNCValues.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCurrentCase.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetCaseGroups.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetSelectedCases.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetSelectedCells.oct"
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
      "${CMAKE_CURRENT_BINARY_DIR}/riGetGridPropertyForSelectedCells.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetPropertyNames.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetWellNames.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetWellStatus.oct"
      "${CMAKE_CURRENT_BINARY_DIR}/riGetWellCells.oct"
      SOURCES
      ${CPP_SOURCES}
      riSettings.h 
    )


    # Copy Octave generated *.oct files to application folder, will make it possible to use Octave functions
    # directly from the location of the ResInsight binaries
    if (true)
        foreach (oct_bin ${OCTAVE_BINARY_OCT_FILES})

            get_filename_component(Filename "${oct_bin}" NAME)

            if(MSVC)
                add_custom_command(TARGET octave_plugins POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${oct_bin}"
                    "${CMAKE_CURRENT_BINARY_DIR}/../ApplicationCode/$<CONFIGURATION>/${Filename}"
                )
            else()
                add_custom_command(TARGET octave_plugins POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${oct_bin}"
                    "${CMAKE_CURRENT_BINARY_DIR}/../ApplicationCode/${Filename}"
                )
            endif()
        endforeach( oct_bin )
    endif()

    # Make ResInsight dependant on Octave, makes it easiser to debug Octave functionality by compiling ResInsight
    add_dependencies(ResInsight octave_plugins)


    if (RESINSIGHT_PRIVATE_INSTALL)
        install(FILES ${OCTAVE_BINARY_OCT_FILES} DESTINATION ${RESINSIGHT_INSTALL_FOLDER})
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

endif (RESINSIGHT_OCTAVE_PLUGIN_QMAKE AND RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE)
    


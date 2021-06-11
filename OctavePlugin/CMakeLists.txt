
# Some of the functionality in this CMakeLists.txt might be easier to maintain if we use a CMake macro to find configuration info for Octave
# This file is currently (2.8.12) not part of default CMake installation
# See http://www.cmake.org/Wiki/CMakeUserFindOctave

set(CPP_SOURCES
    riGetActiveCellProperty.cpp
    riSetActiveCellProperty.cpp
    riGetActiveCellInfo.cpp
    riGetMainGridDimensions.cpp
    riGetNNCConnections.cpp
    riGetNNCPropertyNames.cpp
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
    riSetNNCProperty.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # since the compiler passes the option to the linker, double quoting is necessary
  set (RPATH_COMMAND "-Wl,-rpath,'\\$$ORIGIN'")
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

message (STATUS "Compiling Octave plugins using : ${OCTAVE_MKOCTFILE}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  option(RESINSIGHT_OCTAVE_PLUGIN_QT "Compile Octave plugin using Qt located insided Octave root folder" ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(RESINSIGHT_OCTAVE_PLUGIN_QT)

    message(STATUS "Compiling Octave plugins using custom Qt located at ${OCTAVE_ROOT_DIR}")

    if (EXISTS ${OCTAVE_ROOT_DIR}/qt5)
      SET(OCTAVE_QT_INCLUDE_DIR             ${OCTAVE_ROOT_DIR}/qt5/include)
      SET(OCTAVE_QT_QTCORE_INCLUDE_DIR      ${OCTAVE_ROOT_DIR}/qt5/include/QtCore)
      SET(OCTAVE_QT_QTNETWORK_INCLUDE_DIR   ${OCTAVE_ROOT_DIR}/qt5/include/QtNetwork)
      SET(OCTAVE_QT_LIBRARY_DIR             ${OCTAVE_ROOT_DIR}/qt5/lib)
      SET(OCTAVE_QT_QTCORE_LIB              Qt5Core)
      SET(OCTAVE_QT_QTNETWORK_LIB           Qt5Network)
    else ()
      SET(OCTAVE_QT_INCLUDE_DIR             ${OCTAVE_ROOT_DIR}/include)
      SET(OCTAVE_QT_QTCORE_INCLUDE_DIR      ${OCTAVE_ROOT_DIR}/include/QtCore)
      SET(OCTAVE_QT_QTNETWORK_INCLUDE_DIR   ${OCTAVE_ROOT_DIR}/include/QtNetwork)
      SET(OCTAVE_QT_LIBRARY_DIR             ${OCTAVE_ROOT_DIR}/lib)
      SET(OCTAVE_QT_QTCORE_LIB              QtCore4)
      SET(OCTAVE_QT_QTNETWORK_LIB           QtNetwork4)
    endif(EXISTS ${OCTAVE_ROOT_DIR}/qt5)

else()

  if (Qt5Core_FOUND)
    message(STATUS "Compiling Octave plugins using system Qt5")

    SET(OCTAVE_QT_QTCORE_INCLUDE_DIR    ${Qt5Core_INCLUDE_DIRS})
    SET(OCTAVE_QT_QTNETWORK_INCLUDE_DIR ${Qt5Network_INCLUDE_DIRS})
    SET(OCTAVE_QT_LIBRARY_DIR           ${QT_LIBRARY_DIR})
    SET(OCTAVE_QT_QTCORE_LIB            Qt5Core)
    SET(OCTAVE_QT_QTNETWORK_LIB         Qt5Network)

  endif(Qt5Core_FOUND)

endif(RESINSIGHT_OCTAVE_PLUGIN_QT)

list (APPEND MKOCTFILE_INCLUDE_DIRS ${OCTAVE_QT_INCLUDE_DIR})
list (APPEND MKOCTFILE_INCLUDE_DIRS ${OCTAVE_QT_QTCORE_INCLUDE_DIR})
list (APPEND MKOCTFILE_INCLUDE_DIRS ${OCTAVE_QT_QTNETWORK_INCLUDE_DIR})


# Add socket interface source code folder
list (APPEND MKOCTFILE_INCLUDE_DIRS ${ResInsight_SOURCE_DIR}/ApplicationLibCode/SocketInterface)

list (REMOVE_DUPLICATES MKOCTFILE_INCLUDE_DIRS)
foreach (item ${MKOCTFILE_INCLUDE_DIRS})
    list (APPEND MKOCTFILE_INCLUDE_TEMP -I${item})
endforeach (item)

string( REPLACE ";" " " MKOCTFILE_INCLUDE_TEMP "${MKOCTFILE_INCLUDE_TEMP}" )

# Use special command to avoid double quoting in add_custom_command()
separate_arguments(MKOCTFILE_INCLUDE_COMMAND_STRING WINDOWS_COMMAND "${MKOCTFILE_INCLUDE_TEMP}")


# Clear the list of binary oct files to be produced
set(OCTAVE_BINARY_OCT_FILES)

if (OCTAVE_MKOCTFILE)
    foreach(srcFileName IN LISTS CPP_SOURCES)

        if(NOT IS_ABSOLUTE "${srcFileName}")
            set(srcFileName "${CMAKE_CURRENT_SOURCE_DIR}/${srcFileName}")
        endif()

        get_filename_component(baseFilename "${srcFileName}" NAME_WE)

        set(octFileName "${CMAKE_CURRENT_BINARY_DIR}/${baseFilename}.oct")

        if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
            add_custom_command(
                OUTPUT "${octFileName}"
                COMMAND set "OCTAVE_HOME=${OCTAVE_ROOT_DIR}"
            	COMMAND set "PATH=%OCTAVE_HOME%\\bin;%PATH%"
                COMMAND ${OCTAVE_MKOCTFILE}
                    ${MKOCTFILE_INCLUDE_COMMAND_STRING} 
            		${RPATH_COMMAND} 
                    -L${OCTAVE_QT_LIBRARY_DIR} 
                    -l${OCTAVE_QT_QTCORE_LIB}
                    -l${OCTAVE_QT_QTNETWORK_LIB} 
            		-o "${octFileName}" "${srcFileName}"
                DEPENDS "${srcFileName}"
                COMMENT "===> Generating ${octFileName}"
            )
        elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            add_custom_command(
                OUTPUT "${octFileName}"
                COMMAND ${OCTAVE_MKOCTFILE}
                    ${QT_INCLUDES}
                    ${QT_FRAMEWORKS}
                    -I${ResInsight_SOURCE_DIR}/ApplicationLibCode/SocketInterface
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
                COMMAND OCTAVE_HOME=${OCTAVE_ROOT_DIR}
                    ${OCTAVE_MKOCTFILE}
                    ${MKOCTFILE_INCLUDE_COMMAND_STRING} 
                    ${RPATH_COMMAND}
                    -L${OCTAVE_QT_LIBRARY_DIR} 
                    -l${OCTAVE_QT_QTCORE_LIB}
                    -l${OCTAVE_QT_QTNETWORK_LIB} 
                    -o "${octFileName}" "${srcFileName}"
                DEPENDS "${srcFileName}"
                COMMENT "===> Generating ${octFileName}"
            )
        endif()

        list(APPEND OCTAVE_BINARY_OCT_FILES "${octFileName}")

    endforeach()

# Create depencedy string represeting the full path to all generated oct-files
foreach (item ${CPP_SOURCES})
  string( REPLACE ".cpp" ".oct" item ${item})
    list (APPEND DEPENDENCY_STRING ${CMAKE_CURRENT_BINARY_DIR}/${item})
endforeach (item)

#message("DEPENDENCY_STRING : ${DEPENDENCY_STRING}")
    
add_custom_target(octave_plugins ALL DEPENDS
      ${DEPENDENCY_STRING}
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
                    "${CMAKE_CURRENT_BINARY_DIR}/../ApplicationExeCode/$<CONFIGURATION>/${Filename}"
                )
            else()
                add_custom_command(TARGET octave_plugins POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${oct_bin}"
                    "${CMAKE_CURRENT_BINARY_DIR}/../ApplicationExeCode/${Filename}"
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

endif (OCTAVE_MKOCTFILE)
    


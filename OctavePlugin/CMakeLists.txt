# mkoctfile -I../../QtInstall/Qt-4.7.3/include/QtNetwork -I../../QtInstall/Qt-4.7.3/include/QtCore -I../../QtInstall/Qt-4.7.3/include -L../../QtInstall/Qt-4.7.3/lib -lQtCore -lQtNetwork getEclipseProperty.cpp

set(CPP_SOURCES
  riGetActiveCellProperty.cpp
  riSetActiveCellProperty.cpp
  riGetActiveCellInfo.cpp
  riGetMainGridDimensions.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(RPATH_COMMAND "-Wl,-rpath=${RESINSIGHT_FINAL_INSTALL_PATH} -Wl,-rpath=.")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (QT_LIBRARY_POSTFIX 4)
endif()

set(OCTAVE_BINARY_OCT_FILES)

# On Windows, the Octave plugins are compiled using 32-bit VS2010
# To be able to do so, we need to establish all Qt-related variables for a 32-bit configuration
# In addition, VS2010 32-bit compile environment must be launched
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND CMAKE_CL_64)
    find_program(32BIT_QMAKE 32bitqmake)
    if(32BIT_QMAKE)
        get_filename_component(32BIT_QMAKE_PATH ${32BIT_QMAKE} PATH)    # Get path to 32-bit Qt binary directory
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

    
find_program(MKOCTFILE_EXECUTABLE mkoctfile)
if(NOT MKOCTFILE_EXECUTABLE)
  message(WARNING "Failed to find mkoctfile")
else()

  # Get path to Octave binary directory to be able to build .oct files if Octave is not in path
  get_filename_component(OCTAVE_PATH ${MKOCTFILE_EXECUTABLE} PATH)

  foreach(srcFileName IN LISTS CPP_SOURCES)
  
    if(NOT IS_ABSOLUTE "${srcFileName}")
        set(srcFileName "${CMAKE_CURRENT_SOURCE_DIR}/${srcFileName}")
    endif()
  
    get_filename_component(baseFilename "${srcFileName}" NAME_WE)

    set(octFileName "${CMAKE_CURRENT_BINARY_DIR}/${baseFilename}.oct")
    
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND CMAKE_CL_64)
        # The following line calls the build configuration for x86 VS2010 compiler
        # call "\"%VS100COMNTOOLS%../../VC/vcvarsall.bat\"" x86 
        add_custom_command(
            OUTPUT "${octFileName}"
            COMMAND call "\"%VS100COMNTOOLS%../../VC/vcvarsall.bat\"" x86
            COMMAND ${CMAKE_COMMAND} ARGS -E chdir ${OCTAVE_PATH} ${MKOCTFILE_EXECUTABLE} -I${OCTAVE_QT_QTNETWORK_INCLUDE_DIR}
                -I${OCTAVE_QT_QTCORE_INCLUDE_DIR} -I${OCTAVE_QT_INCLUDE_DIR} ${RPATH_COMMAND}
                -L${OCTAVE_QT_LIBRARY_DIR} -lQtCore${QT_LIBRARY_POSTFIX} -lQtNetwork${QT_LIBRARY_POSTFIX} -o "${octFileName}" "${srcFileName}"
            DEPENDS "${srcFileName}"
            COMMENT "Generating ${octFileName}"
        )
    else()
        add_custom_command(
            OUTPUT "${octFileName}"
            COMMAND ${CMAKE_COMMAND} ARGS -E chdir ${OCTAVE_PATH} ${MKOCTFILE_EXECUTABLE} -I${OCTAVE_QT_QTNETWORK_INCLUDE_DIR}
                -I${OCTAVE_QT_QTCORE_INCLUDE_DIR} -I${OCTAVE_QT_INCLUDE_DIR} ${RPATH_COMMAND}
                -L${OCTAVE_QT_LIBRARY_DIR} -lQtCore${QT_LIBRARY_POSTFIX} -lQtNetwork${QT_LIBRARY_POSTFIX} -o "${octFileName}" "${srcFileName}"
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
  )
endif()

install(FILES ${OCTAVE_BINARY_OCT_FILES} DESTINATION ${RESINSIGHT_FINAL_NAME})

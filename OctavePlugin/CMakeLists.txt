# mkoctfile -I../../QtInstall/Qt-4.7.3/include/QtNetwork -I../../QtInstall/Qt-4.7.3/include/QtCore -I../../QtInstall/Qt-4.7.3/include -L../../QtInstall/Qt-4.7.3/lib -lQtCore -lQtNetwork getEclipseProperty.cpp

set(CPP_SOURCES
  riGetActiveCellProperty.cpp
  riSetActiveCellProperty.cpp
  riGetActiveCellInfo.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(RPATH_COMMAND "-Wl,-rpath=${RESINSIGHT_FINAL_INSTALL_PATH} -Wl,-rpath=.")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (QT_LIBRARY_POSTFIX 4)
endif()

set(OCTAVE_BINARY_OCT_FILES)


find_program(MKOCTFILE_EXECUTABLE mkoctfile)

if(NOT MKOCTFILE_EXECUTABLE)
  message(WARNING "Failed to find mkoctfile")
else()

  foreach(srcFileName IN LISTS CPP_SOURCES)
  
    if(NOT IS_ABSOLUTE "${srcFileName}")
        set(srcFileName "${CMAKE_CURRENT_SOURCE_DIR}/${srcFileName}")
    endif()
  
    get_filename_component(baseFilename "${srcFileName}" NAME_WE)

    set(octFileName "${CMAKE_CURRENT_BINARY_DIR}/${baseFilename}.oct")


    add_custom_command(
        OUTPUT "${octFileName}"
        COMMAND ${MKOCTFILE_EXECUTABLE} -I${QT_QTNETWORK_INCLUDE_DIR} -I${QT_QTCORE_INCLUDE_DIR} -I${QT_INCLUDE_DIR} ${RPATH_COMMAND} -L${QT_LIBRARY_DIR} -lQtCore${QT_LIBRARY_POSTFIX} -lQtNetwork${QT_LIBRARY_POSTFIX} -o "${octFileName}" "${srcFileName}"
        DEPENDS "${srcFileName}"
        COMMENT "Generating ${octFileName}"
    )
  
     list(APPEND OCTAVE_BINARY_OCT_FILES "${octFileName}")
  
  endforeach()
  

  #add_custom_target(oct-files ALL DEPENDS "${OCTAVE_BINARY_OCT_FILES}")

  add_custom_target(octave_plugins ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellProperty.oct" "${CMAKE_CURRENT_BINARY_DIR}/riSetActiveCellProperty.oct" "${CMAKE_CURRENT_BINARY_DIR}/riGetActiveCellInfo.oct")
endif()

install(FILES ${OCTAVE_BINARY_OCT_FILES} DESTINATION ${RESINSIGHT_FINAL_NAME})

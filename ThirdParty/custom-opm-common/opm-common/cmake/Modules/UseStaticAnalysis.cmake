# Add static analysis tests for a given source file

macro(setup_static_analysis_tools)
  find_package(CppCheck)
  if(CMAKE_EXPORT_COMPILE_COMMANDS)
    find_package(ClangCheck)
  else()
    message(STATUS "Disabling clang-check as CMAKE_EXPORT_COMPILE_COMMANDS is not enabled")
  endif()
  if(OPM_COMMON_ROOT)
    set(DIR ${OPM_COMMON_ROOT})
  elseif(OPM_MACROS_ROOT)
    set(DIR ${OPM_MACROS_ROOT})
  else()
    set(DIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
  if(CPPCHECK_FOUND)
    file(COPY        ${DIR}/cmake/Scripts/cppcheck-test.sh
         DESTINATION bin
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif()
  if(CLANGCHECK_FOUND AND CMAKE_EXPORT_COMPILE_COMMANDS)
    configure_file(${DIR}/cmake/Scripts/clang-check-test.sh.in
                   ${CMAKE_BINARY_DIR}/CMakeFiles/clang-check-test.sh)
    file(COPY ${CMAKE_BINARY_DIR}/CMakeFiles/clang-check-test.sh
         DESTINATION bin
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
   endif()
endmacro()

function(add_static_analysis_tests sources includes)
  if(CPPCHECK_FOUND OR (CLANGCHECK_FOUND AND CMAKE_EXPORT_COMPILE_COMMANDS))
    foreach(dep ${${includes}})
      list(APPEND IPATHS -I ${dep})
    endforeach()
    foreach(src ${${sources}})
      if(src MATCHES "TARGET_OBJECTS:")
        string(REGEX REPLACE "\\$<TARGET_OBJECTS:(.*)>" "\\1" TGT ${src})
        get_target_property(src ${TGT} SOURCES)
      endif()
      if(IS_ABSOLUTE ${src})
        file(RELATIVE_PATH name ${PROJECT_SOURCE_DIR} ${src})
      else()
        set(name ${src})
        set(src ${PROJECT_SOURCE_DIR}/${src})
      endif()
      if(CPPCHECK_FOUND)
        if(NOT TEST cppcheck+${name})
          add_test(NAME cppcheck+${name}
                   COMMAND bin/cppcheck-test.sh ${CPPCHECK_PROGRAM} ${src} ${IPATHS}
                   CONFIGURATIONS analyze cppcheck)
        endif()
      endif()
      if(CLANGCHECK_FOUND AND CMAKE_EXPORT_COMPILE_COMMANDS)
        if(NOT TEST clang-check+${name})
          add_test(NAME clang-check+${name}
                   COMMAND bin/clang-check-test.sh ${CLANGCHECK_PROGRAM} ${src}
                   CONFIGURATIONS analyze clang-check)
        endif()
      endif()
    endforeach()
  endif()
endfunction()

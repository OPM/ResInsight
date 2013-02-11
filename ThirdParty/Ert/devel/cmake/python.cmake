macro(add_python_target tgt  PYTHON_INSTALL_PATH ARGN)
  SET(OUT_FILES "")                     
  foreach(file ${ARGN})
    set(OUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.pyc)
    list(APPEND OUT_FILES ${OUT})
    ADD_CUSTOM_COMMAND(  
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.pyc
      COMMAND python -m py_compile 
      ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${file}.py
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMAND mv 
      ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${file}.pyc ${CMAKE_CURRENT_BINARY_DIR}
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}.pyc DESTINATION  ${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_PATH})
  endforeach(file)
list(REMOVE_DUPLICATES OUT_FILES)
ADD_CUSTOM_TARGET(    
  ${tgt} ALL
  DEPENDS ${OUT_FILES}) 
endmacro()

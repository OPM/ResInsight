macro(add_python_target tgt  PYTHON_INSTALL_PATH ARGN)
  SET(OUT_FILES "")                     
  foreach(file ${ARGN})
    set(OUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.pyc)
    list(APPEND OUT_FILES ${OUT})
#------------------------------------------------------    
    ADD_CUSTOM_COMMAND(  
      OUTPUT ${OUT}
      COMMAND ${PROJECT_SOURCE_DIR}/cmake/cmake_pyc
      ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${file}.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PATH}
    )
#------------------------------------------------------    
    if (INSTALL_ERT)                                                           
       install(FILES ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PATH}/${file}.pyc DESTINATION  ${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_PATH})
       install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${file}.py  DESTINATION  ${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_PATH})
    endif() 
  endforeach(file)
list(REMOVE_DUPLICATES OUT_FILES)
ADD_CUSTOM_TARGET(    
  ${tgt} ALL
  DEPENDS ${OUT_FILES}) 
endmacro()

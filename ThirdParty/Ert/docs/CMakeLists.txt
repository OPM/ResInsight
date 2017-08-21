find_package(Sphinx REQUIRED)
if (SPHINX_FOUND)
  set( ERT_DOC_INSTALL_PATH  "" CACHE PATH "Absolute path to install documentation *in addition* to $PREFIX/documentation")
  set( ERT_DOC_EXTERNAL_ROOT "" CACHE PATH "Path to site local ERT documentation")

  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/doc-src")
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/doc-src/_static")
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/user" "${PROJECT_BINARY_DIR}/doc-src/user")

  if (ERT_DOC_EXTERNAL_ROOT)
    EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${ERT_DOC_EXTERNAL_ROOT}" "${PROJECT_BINARY_DIR}/doc-src/external-doc")
    message(STATUS "Adding documentation link ${PROJECT_BINARY_DIR}/doc-src/external-doc -> ${ERT_DOC_EXTERNAL_ROOT}")
    set( ERT_DOC_LINK external-doc/index )
  else()
    set( ERT_DOC_LINK "" )
  endif()

  configure_file(index.rst.in ${PROJECT_BINARY_DIR}/doc-src/index.rst)
  configure_file(conf.py.in ${PROJECT_BINARY_DIR}/doc-src/conf.py)

  if (BUILD_PYTHON)
     add_custom_target(api-doc ALL
       COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/code" "${PROJECT_BINARY_DIR}/doc-src/code"
       COMMAND sphinx-apidoc -e -o doc-src/API/python ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}
       DEPENDS ecl
       WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
  endif()

  add_custom_target(rst-doc ALL
    COMMAND sphinx-build -b html -d doc-src/doctrees doc-src documentation/rst
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    DEPENDS api-doc
    )

else()
  message(STATUS "Sphinx documentation tool not found - documentation not generated")
endif()

INSTALL( DIRECTORY ${PROJECT_BINARY_DIR}/documentation DESTINATION ${CMAKE_INSTALL_PREFIX} )

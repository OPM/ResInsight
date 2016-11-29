set( ERT_DOC_INSTALL_PATH  "" CACHE PATH "Absolute path to install documentation *in addition* to $PREFIX/documentation")
set( ERT_DOC_EXTERNAL_ROOT "" CACHE PATH "Path to site local ERT documentation")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tmp_doc")
EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/user" "${PROJECT_BINARY_DIR}/tmp_doc/user")
EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/code" "${PROJECT_BINARY_DIR}/tmp_doc/code")

if (ERT_DOC_EXTERNAL_ROOT)
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${ERT_DOC_EXTERNAL_ROOT}" "${PROJECT_BINARY_DIR}/tmp_doc/external-doc")
  message(STATUS "Adding documentation link ${PROJECT_BINARY_DIR}/tmp_doc/external-doc -> ${ERT_DOC_EXTERNAL_ROOT}")
  set( ERT_DOC_LINK external-doc/index )
else()
  set( ERT_DOC_LINK "" )
endif()

configure_file(index.rst.in ${PROJECT_BINARY_DIR}/tmp_doc/index.rst)
configure_file(conf.py.in ${PROJECT_BINARY_DIR}/conf.py)

add_custom_target(doc_out ALL
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/script/run-sphinx.py ${PROJECT_BINARY_DIR}/conf.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ${PROJECT_BINARY_DIR}/tmp_doc
  DEPENDS enkf)

INSTALL( DIRECTORY ${PROJECT_BINARY_DIR}/tmp_doc/_build/ DESTINATION ${CMAKE_INSTALL_PREFIX}/documentation )

if (ERT_DOC_INSTALL_PATH)
  INSTALL( DIRECTORY ${PROJECT_BINARY_DIR}/tmp_doc/_build/ DESTINATION ${ERT_DOC_INSTALL_PATH} )
endif()

find_package(Doxygen)
if (DOXYGEN_FOUND)
  message(STATUS "Creating doxygen target")
  if (DOXYGEN_DOT_FOUND)
    message(STATUS "Found graphviz, will run doxygen with graphics")
    set( DOXYGEN_HAVE_DOT "YES" )
  else()
    message(STATUS "Graphviz not found, disabling dot")
    set( DOXYGEN_HAVE_DOT "NO" )
  endif()

  configure_file(doxygen.cfg.in ${PROJECT_BINARY_DIR}/doxygen.cfg)
  add_custom_target(doxy
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doxygen.cfg
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../
    COMMENT "Generating doxygen documentation")
endif()

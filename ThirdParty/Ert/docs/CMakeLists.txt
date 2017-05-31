option( ERT_RST_DOC           "Build RST based documentation"  ON)
option( ERT_DOXY_DOC          "Build doxygen documentation"  ON)
option( ERT_DOXY_GRAPHICAL    "Add graphics to doxygen documentation"  OFF)

if (NOT BUILD_ERT)
  if (ERT_RST_DOC)
    message(WARNING "Turning off ERT_RST_DOC.  Depends on BUILD_ERT.")
    set (ERT_RST_DOC OFF)
  endif()
  if (ERT_DOXY_DOC)
    message(WARNING "Turning off ERT_DOXY_DOC.  Depends on BUILD_ERT.")
    set (ERT_DOXY_DOC OFF)
  endif()
endif()

if (ERT_RST_DOC)
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
       EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/code" "${PROJECT_BINARY_DIR}/doc-src/code")
       add_custom_target(api-doc ALL
         COMMAND sphinx-apidoc -e -o doc-src/API/python ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}
         DEPENDS enkf
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
endif()


if (ERT_DOXY_DOC)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/documentation/doxy")
    set( DOXYGEN_HAVE_DOT "NO" )
    if ( ERT_DOXY_GRAPHICAL)
      if (DOXYGEN_DOT_FOUND)
        set( DOXYGEN_HAVE_DOT "YES" )
      endif() 
    endif()

    if (BUILD_ERT)
        SET( DOXYGEN_INPUT "${PROJECT_SOURCE_DIR}/libanalysis ${PROJECT_SOURCE_DIR}/libconfig ${PROJECT_SOURCE_DIR}/libecl ${PROJECT_SOURCE_DIR}/libecl_well ${PROJECT_SOURCE_DIR}/libeclxx ${PROJECT_SOURCE_DIR}/libenkf ${PROJECT_SOURCE_DIR}/libert_util ${PROJECT_SOURCE_DIR}/libert_utilxx ${PROJECT_SOURCE_DIR}/libgeometry ${PROJECT_SOURCE_DIR}/libjob_queue ${PROJECT_SOURCE_DIR}/librms ${PROJECT_SOURCE_DIR}/libsched")
     else()
        SET( DOXYGEN_INPUT "${PROJECT_SOURCE_DIR}/libecl ${PROJECT_SOURCE_DIR}/libecl_well ${PROJECT_SOURCE_DIR}/libeclxx ${PROJECT_SOURCE_DIR}/libert_util ${PROJECT_SOURCE_DIR}/libert_utilxx ${PROJECT_SOURCE_DIR}/libgeometry")
     endif()

    configure_file(doxygen.cfg.in ${PROJECT_BINARY_DIR}/doxygen.cfg)
    add_custom_target(doxy ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doxygen.cfg
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../
      COMMENT "Generating doxygen documentation"
      DEPENDS enkf)

  endif()
endif()


INSTALL( DIRECTORY ${PROJECT_BINARY_DIR}/documentation DESTINATION ${CMAKE_INSTALL_PREFIX} )

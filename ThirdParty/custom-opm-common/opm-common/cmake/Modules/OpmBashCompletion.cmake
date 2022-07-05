# Installs bash tab completion for a product
macro(opm_add_bash_completion binary)
  option(USE_BASH_COMPLETIONS_DIR
    "Whether to use the new bash completion dir (/usr/share/bash-completion/completions) with load on demand"
    OFF)
  if(USE_BASH_COMPLETIONS_DIR)
    set(_BASH_COMPLETION_FILE ${binary})
    set(_BASH_COMPLETION_INSTALL_DIR ${CMAKE_INSTALL_DATAROOTDIR}/bash-completion/completions)
  else()
    set(_BASH_COMPLETION_FILE ${binary}_bash_completion.sh)
    set(_BASH_COMPLETION_INSTALL_DIR ${CMAKE_INSTALL_SYSCONFDIR}/bash_completion.d)
  endif()
  set(PRODUCT ${binary})
  configure_file(${OPM_MACROS_ROOT}/etc/opm_bash_completion.sh.in ${_BASH_COMPLETION_FILE} @ONLY)
  install(FILES ${PROJECT_BINARY_DIR}/${_BASH_COMPLETION_FILE} DESTINATION ${_BASH_COMPLETION_INSTALL_DIR})
endmacro()

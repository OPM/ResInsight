# Installs bash tab completion for a product
macro(opm_add_bash_completion binary)
  set(PRODUCT ${binary})
  configure_file(${OPM_MACROS_ROOT}/etc/opm_bash_completion.sh.in ${binary}_bash_completion.sh @ONLY)
  install(FILES ${PROJECT_BINARY_DIR}/${binary}_bash_completion.sh DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/bash_completion.d)
endmacro()

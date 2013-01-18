set( source_files plot.c plot_dataset.c  plot_range.c plplot_driver.c plot_driver.c text_driver.c)
set( header_files plot.h plot_dataset.h  plot_range.h plot_const.h plot_driver.h  plplot_driver.h text_driver.h)

add_library( plot ${LIBRARY_TYPE} ${source_files} )

target_link_libraries( plot ert_util ${PLPLOT_LIBRARY} )
#-----------------------------------------------------------------
if (PLOT_INSTALL_PREFIX)
   install(TARGETS plot DESTINATION ${PLOT_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${PLOT_INSTALL_PREFIX}/include)
else()
   install(TARGETS plot DESTINATION lib)
   install(FILES ${header_files} DESTINATION include)
endif()        



set( source_files plot.c plot_dataset.c  plot_range.c plplot_driver.c plot_driver.c text_driver.c)
set( header_files plot.h plot_dataset.h  plot_range.h plot_const.h plot_driver.h  plplot_driver.h text_driver.h)

add_library( plot ${LIBRARY_TYPE} ${source_files} )


target_link_libraries( plot ert_util ${PLPLOT_LIBRARY} )
set_target_properties( plot PROPERTIES VERSION 1.0 SOVERSION 1.0 )
if (USE_RUNPATH)
   add_runpath( plot )
endif()
#-----------------------------------------------------------------
if (INSTALL_ERT)
   install(TARGETS plot DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/plot/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/plot)
   endforeach()
endif()

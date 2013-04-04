set( source_files rms_file.c rms_util.c rms_tag.c rms_type.c rms_tagkey.c rms_stats.c rms_export.c)
set( header_files rms_file.h rms_util.h rms_tag.h rms_type.h rms_tagkey.h rms_stats.h rms_export.h)

add_library( rms ${LIBRARY_TYPE} ${source_files} )
set_target_properties( rms PROPERTIES VERSION 1.0 SOVERSION 1.0 )

#-----------------------------------------------------------------
if (INSTALL_ERT)
   install(TARGETS rms DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/rms/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/rms)
   endforeach()
endif()


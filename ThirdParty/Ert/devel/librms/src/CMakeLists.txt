set( source_files rms_file.c rms_util.c rms_tag.c rms_type.c rms_tagkey.c rms_stats.c rms_export.c)
set( header_files rms_file.h rms_util.h rms_tag.h rms_type.h rms_tagkey.h rms_stats.h rms_export.h)

add_library( rms ${LIBRARY_TYPE} ${source_files} )

#-----------------------------------------------------------------
if (RMS_INSTALL_PREFIX)
   install(TARGETS rms DESTINATION ${RMS_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${RMS_INSTALL_PREFIX}/include)
else()
   install(TARGETS rms DESTINATION lib)
   install(FILES ${header_files} DESTINATION include)
endif()        



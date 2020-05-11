
# Copy Odb Dlls
set(RESINSIGHT_ODB_API_DIR "" CACHE PATH "Path tho the ODB api from Simulia")

if (MSVC)
	# Find all the dlls
	file (GLOB  RI_ALL_ODB_DLLS  ${RESINSIGHT_ODB_API_DIR}/lib/*.dll)
	
	# Strip off the path
	foreach (aDLL  ${RI_ALL_ODB_DLLS})
		 get_filename_component(filenameWithExt ${aDLL} NAME)
		 list(APPEND RI_ODB_DLLS ${filenameWithExt} )
	endforeach(aDLL)

	# Copy to target directory
	#foreach (aDLL ${RI_ODB_DLLS})
    #    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #                       COMMAND ${CMAKE_COMMAND} -E copy_if_different  "${RI_ODB_API_DIR}/lib/${aDLL}"  "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    #endforeach()
	foreach (aDLL ${RI_ODB_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different  
						   ${RESINSIGHT_ODB_API_DIR}/lib/${aDLL}
						   $<TARGET_FILE_DIR:ResInsight>)
    endforeach()

endif(MSVC)

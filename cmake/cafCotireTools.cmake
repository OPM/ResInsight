#
# This macro is used when a target has been created (add_library or add_executable)
# Apply the macro like this
# 
# add_executable(MyProject)
# caf_apply_cotire(MyProject)
#
#
macro (caf_apply_cotire target_name)
  if (COMMAND cotire)

    #message("Applying cotire on target : ${target_name}")

    # disable precompiled headers
    set_target_properties(${target_name} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)

    foreach (fileToExclude ${CAF_COTIRE_EXCLUDE_FILES})
        set_source_files_properties (${fileToExclude} PROPERTIES COTIRE_EXCLUDED TRUE)
    endforeach(fileToExclude)

    cotire(${target_name})
  
    # make sure the unity target is included in the active builds to trigger rebuild before debug
    get_target_property(_unityTargetName ${target_name} COTIRE_UNITY_TARGET_NAME)
    set_target_properties(${_unityTargetName} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD FALSE)
    set_target_properties(${target_name} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  endif()
endmacro (caf_apply_cotire)


#
# This macro is intended to be used in *.cmake files listing a set of source files.
# The specified file will start a new unity source file
#
# Example:
#   set (SOURCE_GROUP_SOURCE_FILES
#   ${CMAKE_CURRENT_LIST_DIR}/RicLaunchUnitTestsFeature.cpp
#   )
#   caf_cotire_start_unity_at_first_item(SOURCE_GROUP_SOURCE_FILES)
#
macro (caf_cotire_start_unity_at_first_item all_files)
  if (COMMAND cotire)
    #message("all_files : ${${all_files}}")
    list(GET ${all_files} 0 first_item_in_list)
    #message("first_item_in_list : ${first_item_in_list}")
    list(APPEND CAF_COTIRE_START_NEW_UNITY_SOURCES
       ${first_item_in_list}
    )
    #message("CAF_COTIRE_START_NEW_UNITY_SOURCES : ${CAF_COTIRE_START_NEW_UNITY_SOURCES}")
  endif(COMMAND cotire)
endmacro (caf_cotire_start_unity_at_first_item)

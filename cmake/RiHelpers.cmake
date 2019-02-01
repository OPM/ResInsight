macro (ri_apply_cotire)
  if (COMMAND cotire)
    cotire(${PROJECT_NAME})
  
    # make sure the unity target is included in the active builds to trigger rebuild before debug
    get_target_property(_unityTargetName ${PROJECT_NAME} COTIRE_UNITY_TARGET_NAME)
    set_target_properties(${_unityTargetName} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD FALSE)
    set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  endif()
endmacro (ri_apply_cotire)

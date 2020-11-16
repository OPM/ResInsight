option (SIBLING_SEARCH "Search sibling directories before system paths" ON)
mark_as_advanced (SIBLING_SEARCH)

macro(create_module_dir_var module)
  if(SIBLING_SEARCH AND NOT ${module}_DIR)
    # guess the sibling dir
    get_filename_component(_leaf_dir_name ${PROJECT_BINARY_DIR} NAME)
    get_filename_component(_parent_full_dir ${PROJECT_BINARY_DIR} DIRECTORY)
    get_filename_component(_parent_dir_name ${_parent_full_dir} NAME)
    #Try if <module-name>/<build-dir> is used
    get_filename_component(_modules_dir ${_parent_full_dir} DIRECTORY)
    set(_clone_dir "${module}")
    if(IS_DIRECTORY ${_modules_dir}/${_clone_dir}/${_leaf_dir_name})
      set(${module}_DIR ${_modules_dir}/${_clone_dir}/${_leaf_dir_name})
    else()
      string(REPLACE ${PROJECT_NAME} ${_clone_dir} _module_leaf ${_leaf_dir_name})
      if(NOT _leaf_dir_name STREQUAL _module_leaf
          AND IS_DIRECTORY ${_parent_full_dir}/${_module_leaf})
        # We are using build directories named <prefix><module-name><postfix>
        set(${module}_DIR ${_parent_full_dir}/${_module_leaf})
      elseif(IS_DIRECTORY ${_parent_full_dir}/${_clone_dir} AND
          EXISTS ${_parent_full_dir}/${_clone_dir}/CMakeCache.txt)
        # All modules are in a common build dir
        set(${module}_DIR "${_parent_full_dir}/${_clone_dir}")
      endif()
    endif()
  endif()
  if(${module}_DIR AND NOT IS_DIRECTORY ${${module}_DIR})
    message(WARNING "Value ${${module}_DIR} passed to variable"
      " ${module}_DIR is not a directory")
  endif()
endmacro()

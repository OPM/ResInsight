function( ert_module_name module module_name lib_path )

    set( osx_file   ${lib_path}/${module_name}.dylib )       
    set( linux_file ${lib_path}/${module_name}.so )       
    set( win_file   ${lib_path}/${module_name}.dll )       

    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
       set( ${module} ${linux_file} PARENT_SCOPE)        
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
       set( ${module} ${osx_file} PARENT_SCOPE)        
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
       set( ${module} ${win_file} PARENT_SCOPE)        
    else()
       message( FATAL_ERROR "Hmmm - which platform is this ??")
    endif()       

endfunction()

if (CMAKE_COMPILER_IS_GNUCC) 
   option (USE_RUNPATH "Embed original dependency paths in installed library" OFF)
   if (USE_RUNPATH)
      set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}") 
      set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
   endif (USE_RUNPATH)
else()
   set(USE_RUNPATH OFF)        
endif()


macro( add_runpath target )
  set_target_properties( ${target} PROPERTIES LINK_FLAGS -Wl,--enable-new-dtags)     
endmacro()
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(vtu2ecl vtu2ecl.cxx)
if (VTK_LIBRARIES)
   target_link_libraries(vtu2ecl ${VTK_LIBRARIES} eclxx_static )
else()
   target_link_libraries(vtu2ecl vtkHybrid eclxx_static )
endif()


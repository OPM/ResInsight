# Copy the .pdb file matching a binary file into a destination folder, if the
# .pdb file is present. Used to get symbols when debugging into prebuilt
# libraries like Qt, as these libraries are distributed with and without .pdb
# files.
#
# Invoke using: cmake -DBINARY_FILE=<file> -DDESTINATION_DIR=<dir> -P
# CopyPdbIfExists.cmake

get_filename_component(_folder "${BINARY_FILE}" DIRECTORY)
get_filename_component(_baseName "${BINARY_FILE}" NAME_WLE)

set(_pdbFile "${_folder}/${_baseName}.pdb")

if(EXISTS "${_pdbFile}")
  file(COPY "${_pdbFile}" DESTINATION "${DESTINATION_DIR}")
endif()

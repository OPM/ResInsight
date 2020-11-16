#
# Module to check whether the file system is case sensitive or not
#
# Sets the following variable:
#
# HAVE_CASE_SENSITIVE_FILESYSTEM   True if the file system honors the case of files

message(STATUS "Checking whether the file system is case-sensitive")
# create a file containing uppercase characters
file(WRITE "${CMAKE_BINARY_DIR}/UPPER" "Foo")

# check if the all-lowercase file with the same name can be opened
set(FooContents "")
if (EXISTS "${CMAKE_BINARY_DIR}/upper")
    file(READ "${CMAKE_BINARY_DIR}/upper" FooContents)
endif()

# remove the file again in order not to have it dangling around...
file(REMOVE "${CMAKE_BINARY_DIR}/UPPER")

# check the contents of the file opened with lower-case. If it is
# empty, the file system is case sensitive.
if ("${FooContents}" STREQUAL "Foo")
  message(STATUS "File system is not case-sensitive")
  set(HAVE_CASE_SENSITIVE_FILESYSTEM 0)
else()
  message(STATUS "File system is case-sensitive")
  set(HAVE_CASE_SENSITIVE_FILESYSTEM 1)
endif()

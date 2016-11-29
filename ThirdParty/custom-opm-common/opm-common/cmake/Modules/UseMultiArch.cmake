# - Multiarch support in object code library directories
#
# This module sets the following variable
#	CMAKE_INSTALL_LIBDIR	     to lib, lib64 or lib/x86_64-linux-gnu
#	                             depending on the platform; use this path
#	                             for platform-specific binaries.
#
# Note that it will override the results of GNUInstallDirs if included after
# that module.

# default if we need to put something in the library directory for a
# component that is *not* multiarch-aware
set (LIBDIR_MULTIARCH_UNAWARE "lib")

# Fedora uses lib64/ for 64-bit systems, Debian uses lib/x86_64-linux-gnu
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  # Debian or Ubuntu?
  if (EXISTS "/etc/debian_version")
	set (_libdir_def "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
  else (EXISTS "/etc/debian_version")
	# 64-bit system?
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	  set (_libdir_def "lib64")
	  set (LIBDIR_MULTIARCH_UNAWARE "${_libdir_def}")
	else (CMAKE_SIZEOF_VOID_P EQUAL 8)
	  set (_libdir_def "lib")
	endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
  endif (EXISTS "/etc/debian_version")
else ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  set (_libdir_def "lib")
endif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")

# let the user override if somewhere else is desirable
set (CMAKE_INSTALL_LIBDIR "${_libdir_def}" CACHE PATH "Object code libraries")
mark_as_advanced (CMAKE_INSTALL_LIBDIR)

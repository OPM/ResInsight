

# Setup the main platform defines
#-----------------------------------------------------
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DCVF_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_definitions(-DCVF_OSX)
endif()


# Compiler specific defines
#-----------------------------------------------------

# Must add debug define manually on GCC
if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}  -D_DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  ")
endif()

# We alwys want Unicode on Win
if (MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
endif()



include(CheckCXXCompilerFlag)

option(CEE_WARNINGS_AS_ERRORS "Make all warnings into errors" ON)

# Compiler flags for GCC
#-----------------------------------------------------
if (CMAKE_COMPILER_IS_GNUCXX)

    # Setup our BASE compile flags
    set(CEE_BASE_CXX_FLAGS  "-Wall -Wextra -pedantic -std=c++11")

    if (CEE_WARNINGS_AS_ERRORS)
        set(CEE_BASE_CXX_FLAGS  "-Werror ${CEE_BASE_CXX_FLAGS}")
    endif()

    # Setup the our STRICT compile flags
    # These are the flags we would like to use on all of our own libraries
	set(CEE_STRICT_CXX_FLAGS  "${CEE_BASE_CXX_FLAGS} -Wconversion -Woverloaded-virtual -Wformat -Wcast-align")

	# Add warning not present on older GCCs
	CHECK_CXX_COMPILER_FLAG("-Wlogical-op" HAS_WLOGICAL_OP)
	if (HAS_WLOGICAL_OP)
		set(CEE_STRICT_CXX_FLAGS "${CEE_STRICT_CXX_FLAGS} -Wlogical-op")
	endif()

endif()


# Compiler flags for MSVC
#-----------------------------------------------------
if (MSVC)

    # Strip out the /W3 flag that Cmake sets by default
    string (REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

    # Setup our BASE compile flags
    set(CEE_BASE_CXX_FLAGS  "")

    if (CEE_WARNINGS_AS_ERRORS)
        set(CEE_BASE_CXX_FLAGS  "/WX ${CEE_BASE_CXX_FLAGS}")
    endif()

    # Setup the our STRICT compile flags
    # These are the flags we would like to use on all of our own libraries
    if (${MSVC_VERSION} LESS 1600)
        set(CEE_STRICT_CXX_FLAGS  "${CEE_BASE_CXX_FLAGS} /W4")
    elseif()
        set(CEE_STRICT_CXX_FLAGS  "${CEE_BASE_CXX_FLAGS} /Wall")
    endif()
    
endif()




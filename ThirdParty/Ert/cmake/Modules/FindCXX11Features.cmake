#
# Module that checks for supported C++11 (former C++0x) features.
#
if(CMAKE_VERSION VERSION_LESS 3.1)
  if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
  if(NOT ERT_WINDOWS)
    set( CMAKE_CXX_FLAGS_main "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
else()
  if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
  endif()
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(CXX11FEATURES_FOUND TRUE)

# Create interface library for common settings
add_library(ResInsightCommonSettings INTERFACE)

# Compiler warnings as errors
if(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(ResInsightCommonSettings INTERFACE /WX)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        target_compile_options(ResInsightCommonSettings INTERFACE -Werror)
    endif()
endif()

if(MSVC)
  # The following warnings are supposed to be used in ResInsight, but
  # temporarily disabled to avoid too much noise warning C4245: 'return':
  # conversion from 'int' to 'size_t', signed/unsigned mismatch warning C4005:
  # Macro redefinition for math constants (M_PI, M_SQRT2 etc)

  # If possible, the following command is supposed to be the final target
  # set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /wd4190
  # /wd4100 /wd4127")

  set(BUILD_FLAGS_FOR_MSVC "/W3 /wd4190 /wd4100 /wd4127")

  message(STATUS "CMAKE_CXX_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION}")

  if(CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 19.38)
    # https://github.com/OPM/ResInsight/issues/10844
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4996")
  endif()

  message(STATUS "BUILD_FLAGS_FOR_MSVC ${BUILD_FLAGS_FOR_MSVC}")
  set_target_properties(
    ResInsightCommonSettings PROPERTIES COMPILE_FLAGS ${BUILD_FLAGS_FOR_MSVC}
  )
endif()


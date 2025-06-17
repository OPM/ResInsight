# Create interface library for common settings
add_library(ResInsightCommonSettings INTERFACE)

option(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS
       "Treat warnings as errors (stops build)" OFF
)
if(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(ResInsightCommonSettings INTERFACE /WX)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        target_compile_options(ResInsightCommonSettings INTERFACE -Werror)
    endif()
endif()
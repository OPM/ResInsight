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
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            # GCC's -Wfree-nonheap-object false-positives on inlined
            # std::vector copies (GCC PR 104475)
            target_compile_options(
                ResInsightCommonSettings INTERFACE
                -Wno-error=free-nonheap-object
            )
        endif()
    endif()
endif()
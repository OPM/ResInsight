add_executable( plot_text plot_text.c )
target_link_libraries( plot_text plot test_util)
add_test( plot_text ${EXECUTABLE_OUTPUT_PATH}/plot_text )

add_executable( plot_plplot plot_plplot.c )
target_link_libraries( plot_plplot plot test_util)
add_test( plot_plplot ${EXECUTABLE_OUTPUT_PATH}/plot_plplot )

add_executable( plot_range plot_range.c )
target_link_libraries( plot_range plot test_util)
add_test( plot_range ${EXECUTABLE_OUTPUT_PATH}/plot_range )

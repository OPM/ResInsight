add_executable(opmi opmi.cpp)
target_link_libraries(opmi opmparser)
install(TARGETS opmi DESTINATION "bin")


execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${BASE_DIR}/tmp_gen/ParserInit.cpp
                        ${BASE_DIR}/ParserInit.cpp)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${BASE_DIR}/tmp_gen/TestKeywords.cpp
                        ${BASE_DIR}/TestKeywords.cpp)


file(GLOB HDRS ${BASE_DIR}/tmp_gen/include/opm/parser/eclipse/Parser/ParserKeywords/*.hpp)

foreach(HDR ${HDRS})
  file(RELATIVE_PATH hdr ${BASE_DIR}/tmp_gen/include/opm/parser/eclipse/Parser/ParserKeywords ${HDR})
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                          ${HDR}
                          ${BASE_DIR}/include/opm/parser/eclipse/Parser/ParserKeywords/${hdr})

endforeach()

foreach (name A B C D E F G H I J K L M N O P Q R S T U V W X Y Z)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            ${BASE_DIR}/tmp_gen/ParserKeywords/${name}.cpp
                            ${BASE_DIR}/ParserKeywords/${name}.cpp)
endforeach()

cmake_minimum_required (VERSION 2.8) 

# Procedure used to extract subset from complete Boost installation
#
# 1) Run command line tool bcp to extract required files for filesystem module
#    http://www.boost.org/doc/libs/1_63_0/tools/bcp/doc/html/index.html#bcp.syntax.options
#    bcp.exe filesystem --boost="c:/local/boost_1_59_0" d:/gitroot/ResInsight/ThirdParty/boost-Subset
#
# 2) Delete the following include folders, as they are large and not required
#    It is probably possible to exclude more folders, but the dependencies for the remaining folders are quite complex
#      boost-Subset/boost/preprocessor
#      boost-Subset/boost/test
#      boost-Subset/boost/typeof

# 3) Delete following source folders, keep folders 'filesystem' and 'system'
#      boost-Subset/libs/chrono 
#      boost-Subset/libs/smart_ptr
#      boost-Subset/libs/test
#      boost-Subset/libs/timer
 
project (boost-Subset) 
 
include_directories( 
. 
) 
 
add_library( ${PROJECT_NAME} 
    libs/filesystem/src/codecvt_error_category.cpp 
    libs/filesystem/src/operations.cpp 
    libs/filesystem/src/path.cpp 
    libs/filesystem/src/path_traits.cpp 
    libs/filesystem/src/portability.cpp 
    libs/filesystem/src/windows_file_codecvt.cpp 
	   
    libs/system/src/error_code.cpp 
) 

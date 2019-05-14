set ( SOURCE_GROUP_HEADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.inl
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridInfoService.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectInfoService.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcResInfoService.h
)

set ( SOURCE_GROUP_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridInfoService.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectInfoService.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcResInfoService.cpp
)

if (MSVC)
	add_definitions(-D_WIN32_WINNT=0x600)		
endif()

add_definitions(-DENABLE_GRPC)

#find_package(OpenSSL REQUIRED)
#set(OPENSSL_LIBS OpenSSL::SSL OpenSSL::Crypto)

# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")
find_package(Protobuf CONFIG 3.0 REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED NO_MODULE)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure gRPC::grpc_unsecure gRPC::gpr)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)	
set(GRPC_LIBRARIES ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF})

if (MSVC)
	set_target_properties(${GRPC_LIBRARIES} PROPERTIES
		MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
		MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
	)
endif(MSVC)

# Cannot use the nice new FindPackage modules since that is CMake 3.12+
if(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})
	message(STATUS "Using Python ${PYTHON_EXECUTABLE}")
endif(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})

# Proto files
set(PROTO_FILES
	"Empty"
	"CaseInfo"
	"GridInfo"
	"ProjectInfo"
	"Commands"
	"ResInfo"
)

set(python_source_path "${CMAKE_SOURCE_DIR}/Python/api")
set(python_examples_path "${CMAKE_SOURCE_DIR}/Python/examples")
set(python_generated_path "${CMAKE_BINARY_DIR}/Python/generated")
set(python_tests_path "${CMAKE_SOURCE_DIR}/Python/tests")

foreach(proto_file ${PROTO_FILES})		
	get_filename_component(rips_proto "${CMAKE_CURRENT_LIST_DIR}/GrpcProtos/${proto_file}.proto" ABSOLUTE)
	get_filename_component(rips_proto_path "${rips_proto}" PATH)

	set(rips_proto_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.cc")
	set(rips_proto_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.h")
	set(rips_grpc_srcs  "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.cc")
	set(rips_grpc_hdrs  "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.h")
	
	add_custom_command(
		OUTPUT "${rips_proto_srcs}" "${rips_proto_hdrs}" "${rips_grpc_srcs}" "${rips_grpc_hdrs}"
		COMMAND ${_PROTOBUF_PROTOC}
		ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated/"
			--cpp_out "${CMAKE_BINARY_DIR}/Generated/"
			-I "${rips_proto_path}"
			--plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
			"${rips_proto}"
		DEPENDS "${rips_proto}"
	)
	
	if (PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})
		set(rips_proto_python "${python_generated_path}/${proto_file}_pb2.py")
		set(rips_grpc_python "${python_generated_path}/${proto_file}_pb2_grpc.py")

		add_custom_command(
			OUTPUT "${rips_proto_python}" "${rips_grpc_python}"
			COMMAND ${PYTHON_EXECUTABLE}
			ARGS -m grpc_tools.protoc
				-I "${rips_proto_path}"
				--python_out "${python_generated_path}"
				--grpc_python_out "${python_generated_path}"
				"${rips_proto}"
			DEPENDS "${rips_proto}"
			VERBATIM
		)

	endif(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})

	list( APPEND GRPC_HEADER_FILES
		  ${rips_proto_hdrs}
		  ${rips_grpc_hdrs}
	)
	
	list( APPEND GRPC_CPP_SOURCES
		  ${rips_proto_srcs}
		  ${rips_grpc_srcs}
	)

	list (APPEND GRPC_PYTHON_GENERATED_SOURCES
	      ${rips_proto_python}
		  ${rips_grpc_python}
	)
endforeach(proto_file)

if (PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})
	set(GRPC_PYTHON_SOURCES
		"${python_source_path}/__init__.py"
		"${python_source_path}/ResInsight.py"
	)
	set(GRPC_PYTHON_EXAMPLES
		"${python_examples_path}/CommandExample.py"
		"${python_examples_path}/GridInfoStreamingExample.py"
		"${python_examples_path}/ResultValues.py"
		"${python_examples_path}/SelectedCases.py"
		"${python_examples_path}/AllCases.py"
	)
	set(GRPC_PYTHON_TESTS
		"${python_tests_path}/test_sample.py"
	)
endif(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})

list ( APPEND GRPC_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list ( APPEND GRPC_CPP_SOURCES ${SOURCE_GROUP_SOURCE_FILES})

source_group( "GrpcInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.cmake )

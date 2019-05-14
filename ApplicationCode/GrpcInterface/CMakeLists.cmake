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

set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure grpc_unsecure gpr)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)	
set(GRPC_LIBRARIES ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF})

message(STATUS ${GRPC_LIBRARIES})

if (MSVC)
	set_target_properties(${GRPC_LIBRARIES} PROPERTIES
		MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
		MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
	)
endif()
# Proto files
set(PROTO_FILES
	"Empty"
	"CaseInfo"
	"GridInfo"
	"ProjectInfo"
	"Commands"
	"ResInfo"
)

foreach(proto_file ${PROTO_FILES})		
	get_filename_component(hw_proto "${CMAKE_CURRENT_LIST_DIR}/GrpcProtos/${proto_file}.proto" ABSOLUTE)
	get_filename_component(hw_proto_path "${hw_proto}" PATH)

	set(hw_proto_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.cc")
	set(hw_proto_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.h")
	set(hw_grpc_srcs  "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.cc")
	set(hw_grpc_hdrs  "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.h")

	add_custom_command(
		OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
		COMMAND ${_PROTOBUF_PROTOC}
		ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated/"
			--cpp_out "${CMAKE_BINARY_DIR}/Generated/"
			-I "${hw_proto_path}"
			--plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
			"${hw_proto}"
		DEPENDS "${hw_proto}"
	)

	list( APPEND GRPC_HEADER_FILES
		 ${hw_proto_hdrs}
		${hw_grpc_hdrs}
	)
	
	list( APPEND GRPC_CPP_SOURCES
		 ${hw_proto_srcs}
		${hw_grpc_srcs}
	)
endforeach(proto_file)

list ( APPEND GRPC_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list ( APPEND GRPC_CPP_SOURCES ${SOURCE_GROUP_SOURCE_FILES})

source_group( "GrpcInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.cmake )

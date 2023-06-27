import os
import subprocess

def call_protoc(proto_path, proto_file_path, output_dir):
    command = f"python -m grpc_tools.protoc --proto_path={proto_path} --python_out={output_dir} --grpc_python_out={output_dir} {proto_file_path}"
    print (command)
    subprocess.call(command, shell=True)

# Get the current working directory
current_dir = os.getcwd()

# Get the absolute path to the relative folder
proto_path = os.path.normpath(os.path.join(current_dir, "../GrpcProtos"))
print ("proto_path :" + proto_path)

output_path = os.path.normpath(os.path.join(current_dir, "../Python/rips/generated"))
print ("output_path :" + output_path)

proto_files = os.listdir(proto_path)
for file_name in proto_files:
   # print(file_name)
   file_extension = os.path.splitext(file_name)[0]
   abs_proto_file_name = os.path.normpath(proto_path + "/"+ file_name)
   call_protoc(proto_path, abs_proto_file_name, output_path)

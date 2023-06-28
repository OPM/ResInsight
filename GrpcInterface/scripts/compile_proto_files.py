import os
import subprocess
import pathlib

def call_protoc(proto_path, proto_file_path, output_dir):
    command = f"python -m grpc_tools.protoc --proto_path={proto_path} --python_out={output_dir} --grpc_python_out={output_dir} {proto_file_path}"
    print(command)
    subprocess.call(command, shell=True)


def generate_proto_files(proto_directory, output_directory):
    proto_files = [file_name for file_name in os.listdir(proto_directory) if os.path.isfile(os.path.join(proto_directory, file_name))]
    for file_name in proto_files:
        abs_proto_file_path = os.path.join(proto_directory, file_name)
        call_protoc(proto_directory, abs_proto_file_path, output_directory)

if __name__ == "__main__":

    current_directory = os.getcwd()

    proto_directory = os.path.normpath(os.path.join(current_directory, "../GrpcProtos"))
    print("proto_directory: " + proto_directory)

    output_directory = os.path.normpath(os.path.join(current_directory, "../Python/rips/generated"))
    print("output_directory: " + output_directory)
    pathlib.Path(output_directory).mkdir(parents=True, exist_ok=True)

    generate_proto_files(proto_directory, output_directory)

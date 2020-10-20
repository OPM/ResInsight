if ( ! (Test-Path -Path "C:/vcpkg") ) {
  cd C:\
  git clone -q https://github.com/Microsoft/vcpkg.git
  cd .\vcpkg

  # Checkout grpc v 1.19.1
  # https://github.com/microsoft/vcpkg/commit/8b4a607c328d100ac9892e2cbcfb9a4b0cf44c10
  git checkout -q 8b4a607c328d100ac9892e2cbcfb9a4b0cf44c10
  .\bootstrap-vcpkg.bat

  # Set instructions to vcpkb to build release only
  Add-Content .\triplets\x64-windows.cmake "set(VCPKG_BUILD_TYPE release)"

  # Install grpc
  .\vcpkg.exe install grpc --triplet x64-windows
}

### Python
$env:PATH = "C:\Python37;C:\Python37\Scripts;$env:PATH"

python --version
python -m pip install --upgrade pip
python -m pip install grpcio-tools
python -m pip install pytest

$env:PATH = "$env:QT5\bin;$env:PATH"

#### BUILD
mkdir cmakebuild
cd .\cmakebuild

cmake -DCMAKE_BUILD_TYPE=Release -DRESINSIGHT_ENABLE_UNITY_BUILD=on -DRESINSIGHT_ENABLE_PRECOMPILED_HEADERS=on "-DCMAKE_PREFIX_PATH=$env:QT5" -DRESINSIGHT_ENABLE_GRPC=true -DRESINSIGHT_GRPC_PYTHON_EXECUTABLE=C:/Python37/python.exe -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -A x64 ..

cmake --build . --target ResInsight --config Release

### TEST
$env:RESINSIGHT_EXECUTABLE="$env:APPVEYOR_BUILD_FOLDER/cmakebuild/ApplicationCode/Release/ResInsight.exe" 

cd ../ApplicationCode/GrpcInterface/Python/rips
python -m pytest --console

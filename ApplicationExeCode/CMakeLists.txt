project(ApplicationExeCode)

# set packaging dir
if(NOT CPACK_PACKAGE_DIRECTORY)
  set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
endif()

if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations"
  )
endif()

# Open GL
find_package(OpenGL)

find_package(
  Qt6
  COMPONENTS Core
             Gui
             OpenGL
             Network
             NetworkAuth
             Widgets
             Xml
             Concurrent
             PrintSupport
             Svg
             Sql
)
set(QT_LIBRARIES
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::Widgets
    Qt6::Xml
    Qt6::Concurrent
    Qt6::PrintSupport
    Qt6::Svg
    Qt6::Sql
)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

if(MSVC)
  # Avoid Qt warnings about macro redefinitions for math constants
  add_definitions(-D_USE_MATH_DEFINES)
endif()

# ##############################################################################
# Defining all the source (and header) files
# ##############################################################################

set(CODE_HEADER_FILES RiaMainTools.h)

set(CODE_SOURCE_FILES RiaMain.cpp RiaMainTools.cpp)

if(RESINSIGHT_ENABLE_GRPC)
  list(APPEND CODE_HEAD_FILES RiaGrpcConsoleApplication.h
       RiaGrpcGuiApplication.h
  )
  list(APPEND CODE_SOURCE_FILES RiaGrpcConsoleApplication.cpp
       RiaGrpcGuiApplication.cpp
  )
endif()

list(APPEND CPP_SOURCES ${CODE_SOURCE_FILES})

# ##############################################################################
# Qt specifics: Moc, ui, resources
# ##############################################################################

# NOTE! Resources in subfolders must append to QRC_FILES using the following
# statement set( QRC_FILES ${QRC_FILES}
# ${CMAKE_CURRENT_SOURCE_DIR}/Resources/myLibrary.qrc PARENT_SCOPE )

set(QRC_FILES ${QRC_FILES} Resources/ResInsight.qrc)

# Adding resource (RC) files for Windows
if(MSVC)
  set(WIN_RESOURCE Resources/ResInsight.rc)
endif()

# ##############################################################################
# Set up the main executable with its source files
# ##############################################################################

# Default behaviour for a Qt application is a console application, resulting in
# a console window always being launced at startup The following statement is
# used to control this behaviour set_target_properties( MY_TARGET PROPERTIES
# LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS") This setting i not honored by Cmake -
# http://public.kitware.com/Bug/view.php?id=14326 We use the following
# workaround described in the bug report ADD_EXECUTABLE(${TARGET_NAME} WIN32
# ${SRC}) See CMake symbol WIN32_EXECUTABLE for details

if(MSVC)
  set(EXE_FILES WIN32)
elseif(APPLE)
  set(EXE_FILES MACOSX_BUNDLE)
endif()
set(EXE_FILES
    ${EXE_FILES}
    ${CPP_SOURCES}
    ${MOC_SOURCE_FILES}
    ${FORM_FILES_CPP}
    ${QRC_FILES}
    ${WIN_RESOURCE}
    ${HEADER_FILES}
    ${REFERENCED_CMAKE_FILES}
    .clang-format
    .clang-tidy
)

qt_add_executable(ResInsight ${EXE_FILES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set_target_properties(
    ResInsight
    PROPERTIES
      COMPILE_FLAGS
      "-Wall -Wno-unused-parameter -Wno-reorder -Wno-parentheses -Wno-switch"
  )

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(
      ResInsight
      PROPERTIES
        COMPILE_FLAGS
        "-Wall -Wno-unused-parameter -Wno-reorder -Wno-parentheses -Wno-switch -Wno-delete-abstract-non-virtual-dtor -Wno-undefined-var-template -Wno-invalid-source-encoding -Wno-enum-compare -Wno-call-to-pure-virtual-from-ctor-dtor -Wno-unused-variable -Wno-unused-private-field -Wno-unused-lambda-capture -Wno-delete-non-abstract-non-virtual-dtor -Wno-braced-scalar-init -Wno-tautological-constant-out-of-range-compare"
    )
  endif()
endif()

if(MSVC)
  # The following warnings are supposed to be used in ResInsight, but
  # temporarily disabled to avoid too much noise warning C4245: 'return':
  # conversion from 'int' to 'size_t', signed/unsigned mismatch warning C4005:
  # Macro redefinition for math constants (M_PI, M_SQRT2 etc)

  # If possible, the following command is supposed to be the final target
  # set_target_properties(ResInsight PROPERTIES COMPILE_FLAGS "/W3 /wd4190
  # /wd4100 /wd4127")

  set(BUILD_FLAGS_FOR_MSVC "/wd4190 /wd4100 /wd4127 /wd4245 /wd4005")

  message(STATUS "BUILD_FLAGS_FOR_MSVC ${BUILD_FLAGS_FOR_MSVC}")
  set_target_properties(
    ResInsight PROPERTIES COMPILE_FLAGS ${BUILD_FLAGS_FOR_MSVC}
  )

endif()

# ##############################################################################
# Application icon for MacOS X bundle
# ##############################################################################

if(APPLE)
  add_custom_command(
    OUTPUT Resources/ResInsight.icns
    COMMAND
      sips -s format icns ${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppLogo48x48.png
      --out ${CMAKE_CURRENT_BINARY_DIR}/Resources/ResInsight.icns
    COMMENT Converting application icon
  )
  add_custom_target(
    ResInsight-icns
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Resources/ResInsight.icns
  )
  add_dependencies(ResInsight ResInsight-icns)
  set_target_properties(
    ResInsight PROPERTIES MACOSX_BUNDLE_ICON_FILE
                          ${CMAKE_CURRENT_BINARY_DIR}/Resources/ResInsight.icns
  )
endif()

if(RESINSIGHT_ENABLE_GRPC)
  list(APPEND THIRD_PARTY_LIBRARIES ${GRPC_LINK_LIBRARIES})

  if(MSVC)
    set_target_properties(
      ResInsight PROPERTIES LINK_FLAGS_RELWITHDEBINFO
                            "/NODEFAULTLIB:MSVCRTD.lib"
    )
  endif()
endif()

# ##############################################################################
# Set up libraries and dependent projects to link with
# ##############################################################################

# According to ivarun rt is needed on OpenSuse, and Fedora. See:
# https://github.com/OPM/ResInsight/pull/7
#
# atomic is needed by openzgy library
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND THIRD_PARTY_LIBRARIES rt atomic)
endif()

set(LINK_LIBRARIES
    ${THIRD_PARTY_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPM_LIBRARIES}
    ${APP_FWK_LIBRARIES}
    ${VIZ_FWK_LIBRARIES}
    ApplicationLibCode
    Commands
    RigGeoMechDataModel
    RifGeoMechFileInterface
)

if(RESINSIGHT_ENABLE_GRPC)
  list(APPEND LINK_LIBRARIES GrpcInterface)
endif()

if(RESINSIGHT_USE_ODB_API)
  add_definitions(-DUSE_ODB_API)
  list(APPEND LINK_LIBRARIES RifOdbReader)
endif()

if(RESINSIGHT_ENABLE_OPENVDS)
  add_definitions(-DUSE_OPENVDS)
endif()

target_link_libraries(ResInsight PRIVATE ${LINK_LIBRARIES})

if(UNIX AND NOT APPLE)
  target_link_libraries(ResInsight PRIVATE xcb)
endif()

# ##############################################################################
# Unity builds
# ##############################################################################

set(UNITY_EXCLUDE_FILES
    # forever is used as variable name, and this symbol is defined by Qt and
    # used in precompiled headers
    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
    qrc_cafAnimControl.cpp qrc_ResInsight.cpp qrc_cafCommandFeatures.cpp
)

if(RESINSIGHT_ENABLE_UNITY_BUILD)
  foreach(fileToExclude ${UNITY_EXCLUDE_FILES})
    set_source_files_properties(
      ${fileToExclude} PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
    )
  endforeach(fileToExclude)
endif()

# ##############################################################################
# Copy Dlls on MSVC
# ##############################################################################

# create an empty library target that will be used to copy files to the build
# folder
add_library(ResInsightDummyTarget EXCLUDE_FROM_ALL empty.cpp)
set_property(TARGET ResInsightDummyTarget PROPERTY FOLDER "FileCopyTargets")

# create a custom target that copies the files to the build folder
foreach(riFileName ${RI_FILENAMES})
  list(
    APPEND
    copyCommands
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_if_different
    ${riFileName}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
endforeach()
add_custom_target(PreBuildFileCopy ${copyCommands})
set_property(TARGET PreBuildFileCopy PROPERTY FOLDER "FileCopyTargets")

# Make ResInsight depend on the prebuild target.
add_dependencies(ResInsight PreBuildFileCopy)

# Generate Python code in a target that is part of ALL_BUILD and depends on
# ResInsight
if(RESINSIGHT_ENABLE_GRPC)
  set(GENERATED_CLASSES_FILE
      ${CMAKE_SOURCE_DIR}/GrpcInterface/Python/rips/generated/generated_classes.py
  )
  add_custom_command(
    OUTPUT ${GENERATED_CLASSES_FILE}
    COMMAND ResInsight ARGS --console --generate ${GENERATED_CLASSES_FILE}
    DEPENDS ResInsight
    COMMENT "Generating ${GENERATED_CLASSES_FILE}"
  )
  add_custom_target(
    RipsGeneratedPythonClasses ALL SOURCES ${GENERATED_CLASSES_FILE}
  )
  add_dependencies(RipsGeneratedPythonClasses ResInsight)

  # Copy the Python folder to build folder. This will ease debugging of Python
  # scripts
  add_custom_command(
    TARGET RipsGeneratedPythonClasses
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/GrpcInterface/Python"
      $<TARGET_FILE_DIR:ResInsight>/Python
  )

endif(RESINSIGHT_ENABLE_GRPC)

# ##############################################################################
# Install
# ##############################################################################

# bundle libraries together with private installation
if(RESINSIGHT_PRIVATE_INSTALL)

  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # put a .exe.local file in the target directory to pick up DLLs from there
    install(
      CODE "exec_program (\"${CMAKE_COMMAND}\" ARGS -E touch \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}ResInsight${CMAKE_EXECUTABLE_SUFFIX}.local)"
    )

    set(RESINSIGHT_FILES ${RI_FILENAMES})

    # install GRPC-related DLLs and generated_classes.py
    if(RESINSIGHT_ENABLE_GRPC)
      set(ZLIB_DLL "$<IF:$<CONFIG:Debug>,zlibd1,zlib1>")
      set(PROTOBUF_DLL "$<IF:$<CONFIG:Debug>,libprotobufd,libprotobuf>")
      set(GRPC_DLL_NAMES ${PROTOBUF_DLL} cares ${ZLIB_DLL} abseil_dll)
      foreach(dllname ${GRPC_DLL_NAMES})
        install(FILES $<TARGET_FILE_DIR:ResInsight>/${dllname}.dll
                DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
        )
      endforeach(dllname ${GRPC_DLL_NAMES})
      install(
        FILES
          ${CMAKE_SOURCE_DIR}/GrpcInterface/Python/rips/generated/generated_classes.py
        DESTINATION ${RESINSIGHT_INSTALL_FOLDER}/Python/rips/generated
      )
    endif()

    # Boost
    find_package(Boost REQUIRED filesystem system)

    get_property(
      _filepath
      TARGET "Boost::filesystem"
      PROPERTY LOCATION_RELEASE
    )

    # The location of Boost is based on the file structure as installed by vcpkg
    # The DLLs are located in the /bin folder
    get_filename_component(_dir ${_filepath} PATH)
    string(REPLACE "/lib" "/bin" _dir ${_dir})

    # Use file clobbing, as the dlls are decorated with local compiler info
    file(GLOB RI_BOOST_DLLS ${_dir}/boost_filesystem*.dll)

    install(
      FILES ${RI_BOOST_DLLS}
      DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
      CONFIGURATIONS Debug Release RelWithDebInfo
    )

    # CRT
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
    set(CMAKE_INSTALL_OPENMP_LIBRARIES ON)
    include(InstallRequiredSystemLibraries)
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
            DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
    )

  endif()

  install(
    TARGETS ResInsight
    BUNDLE DESTINATION ${RESINSIGHT_INSTALL_FOLDER} COMPONENT Runtime
    RUNTIME DESTINATION ${RESINSIGHT_INSTALL_FOLDER} COMPONENT Runtime
  )

  install(FILES ${RESINSIGHT_FILES} DESTINATION ${RESINSIGHT_INSTALL_FOLDER})

  if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    # Add a post-build command to run windeployqt. This makes it possible to
    # debug the binary from a debugger without any manual steps. This also fixes
    # an issue related to the Install() command and usage of windeployqt as part
    # of installation. The installation tried to use windeployqt on the
    # executable in the build directory, which did not find the Qt libraries.
    #
    # TODO: This command copies all required dlls, so we should probably not use
    # the Install() command to copy external dlls dependencies.
    #
    # The statements " >NUL 2>NUL" are used to suppress text output from
    # windeployqt. ">NUL" suppresses standard output. "2>NUL" suppresses
    # standard error.
    #
    add_custom_command(
      TARGET ResInsight
      POST_BUILD
      COMMAND
        ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:ResInsight>
        "$<IF:$<CONFIG:Debug>,--debug,--release>" --no-translations >NUL 2>NUL
      COMMENT
        "Running windeployqt to deploy Qt dependencies to the build folder, required by install()"
    )

    # Running windeployqt to deploy Qt dependencies to the install folder"
    install(
      TARGETS ResInsight
      DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
      RUNTIME_DEPENDENCIES PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
      POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
    )
  endif()

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/resinsight
      DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
      PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
    )
  endif()

else(RESINSIGHT_PRIVATE_INSTALL)
  # binaries go in /usr/bin
  install(TARGETS ResInsight DESTINATION bin)
  # license go in /usr/share/doc
  install(FILES ${RESINSIGHT_LICENSE_FILES} DESTINATION share/doc/ResInsight)
  # no bundled libraries for system install application icon
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppLogo48x48.png
    DESTINATION share/icons/hicolor/48x48/apps
    RENAME ResInsight.png
  )
  # desktop environment icon; remember to call `update-desktop-database` in
  # package post-install scripts
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resinsight.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/resinsight.desktop @ONLY
  )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/resinsight.desktop
          DESTINATION share/applications
  )
endif(RESINSIGHT_PRIVATE_INSTALL)

# ##############################################################################
# Optional install of OpensSSL 3 for required platforms like RHEL8
# ##############################################################################

set(CUSTOM_OPENSSL_FILES libcrypto.so libcrypto.so.3 libssl.so libssl.so.3)
foreach(file ${CUSTOM_OPENSSL_FILES})
  if(EXISTS
     "${CMAKE_BINARY_DIR}/vcpkg_installed_custom/x64-linux-dynamic/lib/${file}"
  )
    install(
      FILES
        ${CMAKE_BINARY_DIR}/vcpkg_installed_custom/x64-linux-dynamic/lib/${file}
      DESTINATION lib64
    )
  endif()
endforeach()

# ##############################################################################
# Installation packaging
# ##############################################################################

if(Qt6Widgets_VERSION VERSION_LESS 6.5.0)
  message("Detected Qt version ${Qt6Widgets_VERSION}")
  message(
    "Install target is not complete as Qt 6.5 is required to use qt_generate_deploy_app_script()"
  )
else()
  qt_generate_deploy_app_script(
    TARGET ResInsight OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR
    NO_TRANSLATIONS
  )
  install(SCRIPT ${deploy_script})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CPACK_GENERATOR TGZ)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CPACK_GENERATOR ZIP)
endif()

# Handling of system name on Windows
if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  if(CMAKE_CL_64)
    set(CPACK_SYSTEM_NAME win64)
  else()
    set(CPACK_SYSTEM_NAME win32)
  endif()
endif()

# Append el5 when compiled on RHEL5 and el6 if compiled on RHEL6
string(REGEX MATCH "el[6,7,8]?" RESINSIGHT_RHEL_SYSTEM_NAME ${CMAKE_SYSTEM})

set(RESINSIGHT_PACKAGE_NAME "ResInsight")

set(RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}-${STRPRODUCTVER}")

if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "")
  set(RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_odb")
endif()

if(NOT ${OCTAVE_VERSION_STRING} EQUAL "")
  set(RESINSIGHT_PACKAGE_NAME
      "${RESINSIGHT_PACKAGE_NAME}_oct-${OCTAVE_VERSION_STRING}"
  )
endif()

# Append el5 when compiled on RHEL5 and el6 if compiled on RHEL6
if(NOT "${RESINSIGHT_RHEL_SYSTEM_NAME}" STREQUAL "")
  set(RESINSIGHT_PACKAGE_NAME
      "${RESINSIGHT_PACKAGE_NAME}_${RESINSIGHT_RHEL_SYSTEM_NAME}"
  )
else()
  set(RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_${CPACK_SYSTEM_NAME}")
endif()

# message("RESINSIGHT_PACKAGE_NAME : " ${RESINSIGHT_PACKAGE_NAME})
set(CPACK_PACKAGE_FILE_NAME ${RESINSIGHT_PACKAGE_NAME})

include(CPack)

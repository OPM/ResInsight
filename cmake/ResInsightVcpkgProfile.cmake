# Select an alternate vcpkg-configuration.json without renaming/copying any
# tracked file. The vcpkg toolchain
# (ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake) defaults
# VCPKG_MANIFEST_DIR to CMAKE_SOURCE_DIR only when it is not already defined,
# and it must see the final value before project() triggers toolchain-file
# processing. This file must therefore be include()'d from the root
# CMakeLists.txt before project() is called -- code that runs after project()
# is too late.
#
# A profile's vcpkg-configuration file is expected at
# "vcpkg-configuration-${RESINSIGHT_VCPKG_PROFILE}.json" in the source root,
# e.g. vcpkg-configuration-rhel8.json for -DRESINSIGHT_VCPKG_PROFILE=rhel8.
# vcpkg.json is copied alongside it so both manifest files live in the same
# directory, as vcpkg requires; the copy is regenerated on every configure so it
# can never drift from the root vcpkg.json.
set(RESINSIGHT_VCPKG_PROFILE
    ""
    CACHE
      STRING
      "Optional vcpkg-configuration profile, e.g. 'rhel8' to use vcpkg-configuration-rhel8.json"
)
if(RESINSIGHT_VCPKG_PROFILE)
  set(_resinsight_vcpkg_profile_config
      "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg-configuration-${RESINSIGHT_VCPKG_PROFILE}.json"
  )
  if(NOT EXISTS "${_resinsight_vcpkg_profile_config}")
    message(
      FATAL_ERROR "RESINSIGHT_VCPKG_PROFILE='${RESINSIGHT_VCPKG_PROFILE}' but "
                  "${_resinsight_vcpkg_profile_config} does not exist"
    )
  endif()

  set(_resinsight_vcpkg_manifest_dir
      "${CMAKE_BINARY_DIR}/vcpkg-manifest-${RESINSIGHT_VCPKG_PROFILE}"
  )
  file(MAKE_DIRECTORY "${_resinsight_vcpkg_manifest_dir}")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json"
    "${_resinsight_vcpkg_manifest_dir}/vcpkg.json" COPYONLY
  )
  configure_file(
    "${_resinsight_vcpkg_profile_config}"
    "${_resinsight_vcpkg_manifest_dir}/vcpkg-configuration.json" COPYONLY
  )

  # overlay-ports/overlay-triplets entries (e.g.
  # "ThirdParty/vcpkg-overlay-ports") are resolved by vcpkg relative to the
  # directory vcpkg-configuration.json lives in. Since the copy above now lives
  # under the build tree instead of the source root, rewrite those repo-relative
  # "ThirdParty/..." paths to absolute paths so they still resolve to the real
  # source tree.
  file(READ "${_resinsight_vcpkg_manifest_dir}/vcpkg-configuration.json"
       _resinsight_vcpkg_profile_json
  )
  string(REPLACE "\"ThirdParty/" "\"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/"
                 _resinsight_vcpkg_profile_json
                 "${_resinsight_vcpkg_profile_json}"
  )
  file(WRITE "${_resinsight_vcpkg_manifest_dir}/vcpkg-configuration.json"
       "${_resinsight_vcpkg_profile_json}"
  )

  set(VCPKG_MANIFEST_DIR
      "${_resinsight_vcpkg_manifest_dir}"
      CACHE PATH "" FORCE
  )
endif()

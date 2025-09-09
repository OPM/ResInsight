# OPM-Common Update Procedure

## CREATE_OPM_COMMON_KEYWORDS

The `CREATE_OPM_COMMON_KEYWORDS` flag is a CMake option used to regenerate C++ code for Eclipse keywords from JSON definition files in the opm-common library.

### Purpose

ResInsight uses a subset of the opm-common library for parsing Eclipse simulation files. The Eclipse keywords are defined in JSON files within opm-common, and C++ parser code is generated from these definitions using the `genkw` tool.

### When to Use

Set `CREATE_OPM_COMMON_KEYWORDS=ON` when:
- Updating to a newer version of opm-common
- New Eclipse keywords have been added to opm-common JSON files
- Existing keyword definitions have changed

### Procedure in opm-common fork

- ResInsight uses a fork of opm-common located at https://github.com/CeetronSolutions/opm-common
- Current branch used by ResInsight is https://github.com/CeetronSolutions/opm-common/tree/development-01 

1. Checkout the desired branch from the forked repository, usually `master`

2. Create a new branch for your changes, e.g., `development-02`

3. Apply clang-format on the following files EGrid, EInit, ERst, both header and cpp

4. Cherry-pick commits from the previous used branch

### Procedure in ResInsight main repository

1. **Enable the flag**: Set `CREATE_OPM_COMMON_KEYWORDS=ON` in the main CMakeLists.txt (line 536). Configure the project using CMake to apply the change.

2. **Build the solution**: When this flag is enabled, opm-common is added to the build solution and the keyword generation tools are compiled

3. **Generate keywords**: Build a test target (e.g., CarfinTest.exe) to trigger C++ code generation using the `genkw` tool. The test target does no compile completely, but the generation step will complete successfully.

4. **Copy generated files**: From the build directory `build/ThirdParty/custom-opm-common/opm-common/`, copy:
   - `ParserInit.cpp` and `config.h` to `ThirdParty/custom-opm-common/generated-opm-common/`
   - The entire `ParserKeywords/` folder to `ThirdParty/custom-opm-common/generated-opm-common/`
   - The entire `include/` folder to `ThirdParty/custom-opm-common/generated-opm-common/`

5. **Disable the flag**: Set `CREATE_OPM_COMMON_KEYWORDS=OFF` to return to normal build mode

6. **Rebuild**: When disabled, the build uses the pre-generated files from `generated-opm-common/` instead of including the full opm-common build

### Important Notes

- When changing this flag, you must reopen the Visual Studio project
- The flag is normally set to `OFF` to avoid the overhead of building opm-common during regular development
- The generated files in `generated-opm-common/` should be committed to the repository after updates
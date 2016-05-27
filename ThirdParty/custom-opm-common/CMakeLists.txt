cmake_minimum_required (VERSION 2.8)

project (custom-opm-common)

include_directories(
       ${CMAKE_CURRENT_SOURCE_DIR}/opm-common
)

#include(opm-common/CMakeLists_files.cmake)

list (APPEND MAIN_SOURCE_FILES
      opm-common/opm/common/data/SimulationDataContainer.cpp
      opm-common/opm/common/OpmLog/CounterLog.cpp
      opm-common/opm/common/OpmLog/EclipsePRTLog.cpp
      opm-common/opm/common/OpmLog/LogBackend.cpp
      opm-common/opm/common/OpmLog/Logger.cpp
      opm-common/opm/common/OpmLog/LogUtil.cpp
      opm-common/opm/common/OpmLog/OpmLog.cpp
      opm-common/opm/common/OpmLog/StreamLog.cpp
      opm-common/opm/common/OpmLog/TimerLog.cpp
)


add_library( ${PROJECT_NAME}
       ${MAIN_SOURCE_FILES}
)


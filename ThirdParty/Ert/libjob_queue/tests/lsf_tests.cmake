# The testing of the lsf submit capabilities is quite troublesome for
# two reasons, and therefor by default disabled:
#
#
#  1. The shell based LSF commands require that user running the
#     bsub/bjobs/bxxx command has passwordless ssh configured to log in
#     to the lsf server. When the build and testing is run as a common
#     'jenkins' user this becomes difficult.
#
#  2. Submitting through the lsf library requires that the build/test
#     server actually is a LIM host; which it typically is not.
#
#-----------------------------------------------------------------
#
# This should be a space separated list of servers which will be 
# tried out when testing the LSF submit capability. The test program
# will interpret the special strings 'NULL' and 'LOCAL' as follows:
#
#   NULL:  Submit using the linked in library functions.
#   LOCAL: Submit using shell commands on the current server
#
set(LSF_SERVER "" CACHE STRING  "List of LSF servers for testing")

if (HAVE_LSF_LIBRARY)
   add_executable( job_lsf_test job_lsf_test.c )
   target_link_libraries( job_lsf_test job_queue util test_util )
   add_test( job_lsf_test ${EXECUTABLE_OUTPUT_PATH}/job_lsf_test )
endif()



if (HAVE_LSF_LIBRARY)
   add_executable( job_lsb job_lsb.c )
   target_link_libraries( job_lsb job_queue util test_util )
   add_test( job_lsb ${EXECUTABLE_OUTPUT_PATH}/job_lsb )
endif()

add_executable( job_lsf_remote_submit_test job_lsf_remote_submit_test.c )
target_link_libraries( job_lsf_remote_submit_test job_queue util test_util )

add_executable( job_lsf_library_submit_test job_lsf_library_submit_test.c )
target_link_libraries( job_lsf_library_submit_test job_queue util )

add_executable( job_program job_program.c )

if (LSF_SERVER)
   add_test( job_lsf_remote_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_lsf_remote_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_program ${LSF_SERVER} NULL LOCAL)
else()
   add_test( job_lsf_remote_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_lsf_remote_submit_test ${EXECUTABLE_OUTPUT_PATH}/job_program NULL LOCAL)
endif()
set_property( TEST job_lsf_remote_submit_test PROPERTY LABELS StatoilData)

# The test program is installed - actually running the test must be
# handled completely on the outside of this build system.
if (INSTALL_ERT)
   install(TARGETS job_program job_lsf_library_submit_test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

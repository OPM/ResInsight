import os
import socket
import sys
import ert.job_queue.driver as driver

#################################################################
# Only a quite few of the Linux computers in Statoil are proper LSF
# nodes, in the sense that they can talk directly to the LSF
# demons. When using LSF from a computer which is not properly part of
# the LSF cluster you must first ssh to a node which can serve as LSF
# server, and then issue the LSF commands there. This is controlled by
# the LSF_SERVER option in the LSF driver.
#
# In this configuration file the LSF server to use is determined by
# using the two first characters from the name of the submitting host
# as a lookup key in the server_list dictionary. 
#
# If your workstation has proper access to LSF you can set the
# environment variable LOCAL_LSF; in that case the lsf_server variable
# will be left at None and the LSF driver instance will issue the LSF
# commands directly at the calling host without going through ssh.
#
# Observe that the ssh-based scheme requires that you have
# passwordless login to the server used as lsf server.
#################################################################

server_list = { "be"  : "be-grid01.be.statoil.no",
                "st"  : "st-grid01.st.statoil.no",
                "tr"  : "tr-grid01.tr.statoil.no",
                "stj" : "tr-grid01.tr.statoil.no",
                "rio" : "rio-grid01.rio.statoil.no" }


def get_lsf_server():
    if os.getenv("LOCAL_LSF"):
        # The user has set the LOCAL_LSF environment variable -
        # signalling that she has access to a proper LSF node.
        return None
    else:
        host = socket.gethostname()
        host_prefix = host.split("-")[0]
        lsf_server = server_list.get( host_prefix , None)
        # This will silently return None if no appropriate LSF server
        # is found. In that case things will blow up at a later stage
        # if/when someone tries to use the invalid lsf server.
        return lsf_server


# The command used to run ECLIPSE. The executable will be called with
# commandline arguments: version data_file num_cpu
ecl_cmd              = "/project/res/etc/ERT/Scripts/run_eclipse.py"

# The ECLIPSE version which will be used, by default.
ecl_version          = "2010.2"

# The resource request passed to the LSF server. In practice every god-damn compute node
# in Statoil will satisfy these needs, so it could just be left as None.
lsf_resource_request = "select[cs && x86_64Linux] rusage[ecl100v2000=1:duration=5]"

lsf_queue            = "normal"
rsh_command          = "/usr/bin/ssh"

driver_options = { driver.LSF_DRIVER   : [("LSF_QUEUE"    , lsf_queue),
                                          ("LSF_RESOURCE" , lsf_resource_request),
                                          ("LSF_SERVER"   , get_lsf_server())],
                   driver.RSH_DRIVER   : [("RSH_COMMAND"  , rsh_command)],
                   driver.LOCAL_DRIVER : []}

driver_type = driver.LSF_DRIVER




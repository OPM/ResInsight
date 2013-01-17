# The SCons build/configuration file global_config.py will look for a
# file local_config.py contains a few compile-time switches. The
# local_config py file should be created locally at the site and is
# therefor not included in version control system.
#
# If the build system can not locate the local_config.py file it will
# instead load this file: local_config_DEFAULT.py. The defaults listed
# in this file should be a reasonable start to compile the source, but
# the resulting executable will probably not be very usable.
#
# To create a local configuration is quite simple:
#
#  1. Copy local_config_DEFAULT.py -> local_config.py
#
#  2. Edit the content of local_config.py
#
# Observe that local_config.py is a Python source file. When you
# create a personal local_config.py you must assign a value to all the
# variables in this file; altough they can of course keep the default
# value present here.


SITE_CONFIG_FILE    = ""
INCLUDE_LSF         = False
LSF_INCLUDE_PATH    = ""
LSF_LIB_PATH        = ""
g2c                 = False
M64                 = True
PLPLOT_INCLUDE_PATH = "/usr/include"
PLPLOT_LIB_PATH     = "/usr/lib"


#-----------------------------------------------------------------
# Here follows a brief description of the different configuration
# variables:
#
# SITE_CONFIG_FILE: When ERT starts it will look for a site-wide
#   configuration file. The path to this configuration file is
#   compiled into the binary, and should be set with the build
#   variable SITE_CONFIG_FILE
#
#   It is also possible to use an environment variable to specify the
#   location of the site wide configuration file, see the function
#   enkf_main_bootstrap in libenkf/src/enkf_main.c.
#
#   In the directory etc-example/ERT there is an example of a possible
#   site wide configuration file.
#
#
# LSF Configuration: LSF is the Load Sharing Facility from Platform
#   Computing, this is a queue system. If you have access to LSF at
#   your site you should set INCLUDE_LSF to True and then subsequently
#   set LSF_INCLUDE_PATH and LSF_LIB_PATH to point to the location of
#   the LSF header files and the lsf libraries. If you do not have
#   access to LSF at your site you can just set INCLUDE_LSF to False
#   and forget about the other LSF variables.
#
#
# g2c: When linking with old versions of lapack you need some
#   additional Fortran runtime libraries, these are included if you
#   set g2c to True.
#
# 
# M64: Should the -m64 switch be used when compiling? Observe that ERT
#   has been developed, tested and used exclusively on 64 bit Linux,
#   so 32 bit is uncharted territory.
#
# 
# PLPLOT: The library PLPLOT is used by ERT to create simple plots,
#   and also by the application libecl/applications/ens_plot.c. You
#   should set the variables PLPLOT_INCLUDE_PATH and PLPLOT_LIB_PATH
#   to point to the include directory and the lib directory of the
#   PLPLOT installation. Observe that the path to the PLPLOT library
#   is embedded into the binary with RPATH.

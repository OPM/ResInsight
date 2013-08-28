How to develop/run the GUI:

You can run the gui in two ways:

1a There are CMakeLists.txt files all the ways through the gui filesystem,
   and if you build the ert distribution you will get file hierarchy
   xxxx/build/python/ert_gui beside the xxxx/build/python/ert hierarchy.

   This is the way the gui should be organized when installed, to invoke the
   gui this way:

       gert -v xxxx/build <config_file>

   Observe that the 'xxxx' part must be an ABSOLUTE path, otherwise the
   frontend script will search in the default installation directory.


1b Alternatively you can use the gert command to invoke the gui in the source
   directory:

       gert -v  xxxx/devel/python <config_file>

   Observe only one 'python' above.



2. You can invoke gui as a python module directly from the source, without
   going through the gert frontend script. This requires that set several
   environment variables:

      ERT_SITE_CONFIG -> /project/res/etc/ERT/site-config
      ERT_SHARE_PATH  -> xxxx/devel/libenkf/applications/ert_gui/share
      LD_LIBRARY_PATH -> xxxx/build/lib64

   And in addition you must source the local_csh file in
   xxxx/devel/python/test to set the PYTHONPATH correctly. 


About the ERT_SHARE_PATH variable: the correct value for the ERT_SHARE_PATH
variable is currently xxxx/devel/libenkf/applications/ert_gui/share, so if
you are using alternative 2 you get this one correctly, whereas if you use
alternative 1 it will be set to a generally incorrect value by the gert
frontend script.

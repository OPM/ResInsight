This directory is meant as a holding place for test data for the ERT
project. The local/ directory should contain test-data which is
checked in an distributed as part of the solution. 

In addition many of the tests expect to find a Statoil/ subdirectory
in the current directory. This directory should link to Statoil
internal test data. This data is currently located on the enkf disk in
Bergen; before you can start using the Statoil specific test data you
must add the following symlink:

    ln -s <enkf_disk_in_Bergen>/ErtTestData Statoil



Command line:
git apply --reject --whitespace=fix patches/fix-synthetic-odb-cases.patch

Description:
Will fix reading of special Eclipse cases related to synthetic ODB test cases. Required for regression testing.

Patch is created by :
git format-patch -1 <sha>

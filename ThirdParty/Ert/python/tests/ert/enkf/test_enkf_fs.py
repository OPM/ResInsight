import os
from ert.enkf import EnkfFs
from ert.enkf import EnKFMain
from ert.enkf.enums import EnKFFSType
from ert.test import ExtendedTestCase , TestAreaContext
from ert.test import ErtTestContext


class EnKFFSTest(ExtendedTestCase):
    def setUp(self):
        self.mount_point = "storage/default"
        self.config_file = self.createTestPath("Statoil/config/with_data/config")


    def test_id_enum(self):
        self.assertEnumIsFullyDefined(EnKFFSType, "fs_driver_impl", "libenkf/include/ert/enkf/fs_types.h")


    def test_create(self):
        with TestAreaContext("create_fs") as work_area:
            work_area.copy_parent_content(self.config_file)

            self.assertTrue(EnkfFs.exists(self.mount_point))
            fs = EnkfFs(self.mount_point)
            self.assertEqual(1, fs.refCount())
            fs.umount()

            self.assertFalse(EnkfFs.exists("newFS"))
            arg = None
            fs = EnkfFs.createFileSystem("newFS", EnKFFSType.BLOCK_FS_DRIVER_ID, arg)
            self.assertTrue(EnkfFs.exists("newFS"))
            self.assertTrue( fs is None )

            with self.assertRaises(IOError):
                version = EnkfFs.diskVersion("does/not/exist")

            version = EnkfFs.diskVersion("newFS")
            self.assertTrue( version >= 106 )


    def test_create2(self):
        with TestAreaContext("create_fs2") as work_area:
            work_area.copy_parent_content(self.config_file)

            new_fs = EnkfFs.createFileSystem("newFS", EnKFFSType.BLOCK_FS_DRIVER_ID, mount = True)
            self.assertTrue( isinstance( new_fs , EnkfFs ))



    def test_throws(self):
        with self.assertRaises(Exception):
            fs = EnkfFs("/does/not/exist")


    def test_refcount(self):
        with ErtTestContext("TEST", self.config_file) as testContext:
            ert = testContext.getErt()
            self.assertTrue(isinstance(ert, EnKFMain))

            fsm = ert.getEnkfFsManager()
            fs = fsm.getCurrentFileSystem()
            self.assertEqual(2, fs.refCount())

            fs.umount()

            with self.assertRaises(AssertionError):
                fs.refCount()

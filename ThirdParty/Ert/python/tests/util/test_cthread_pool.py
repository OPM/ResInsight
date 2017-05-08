import ctypes
import ecl
from ecl.test import ExtendedTestCase
from ecl.util import CThreadPool, startCThreadPool

TEST_LIB = ecl.load("libert_util")


class CThreadPoolTest(ExtendedTestCase):
    def test_cfunc(self):
        with self.assertRaises(TypeError):
            func = CThreadPool.lookupCFunction("WRONG-TYPE", "no-this-does-not-exist")

        with self.assertRaises(AttributeError):
            func = CThreadPool.lookupCFunction(TEST_LIB, "no-this-does-not-exist")

    def test_create(self):
        pool = CThreadPool(32, start=True)
        job = CThreadPool.lookupCFunction(TEST_LIB, "thread_pool_test_func1")
        arg = ctypes.c_int(0)

        N = 256
        for i in range(N):
            pool.addTask(job, ctypes.byref(arg))
        pool.join()
        self.assertEqual(arg.value, N)

    def test_context(self):
        N = 256
        arg = ctypes.c_int(0)
        job = CThreadPool.lookupCFunction(TEST_LIB, "thread_pool_test_func1")
        with startCThreadPool(16) as tp:
            for i in range(N):
                tp.addTask(job, ctypes.byref(arg))
        self.assertEqual(arg.value, N)

    def test_add_task_function(self):
        pool = CThreadPool(32, start=True)
        pool.addTaskFunction("testFunction", TEST_LIB, "thread_pool_test_func1")

        arg = ctypes.c_int(0)
        task_count = 256
        for i in range(task_count):
            pool.testFunction(ctypes.byref(arg))

        pool.join()
        self.assertEqual(arg.value, task_count)

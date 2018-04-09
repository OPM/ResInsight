import time
from ecl.util.util import ThreadPool
from ecl.util.util.thread_pool import Task
from tests import EclTest


class ThreadPoolTest(EclTest):


    def sleepTask(self, *args, **kwargs):
        time.sleep(args[0])

    def numberer(self, index, result):
        result[index] = True

    def test_pool_creation(self):
        pool = ThreadPool(4)

        self.assertEqual(4, pool.poolSize())

        def noop(*args, **kwargs):
            pass

        pool.addTask(noop)
        self.assertEqual(1, pool.taskCount())

        pool.addTask(noop, 1, 2, 3)
        self.assertEqual(2, pool.taskCount())

        pool.addTask(noop, 1, 2, 3, name="name", group="group", purpose="porpoise")
        self.assertEqual(3, pool.taskCount())

        self.assertEqual(pool.runningCount(), 0)
        self.assertEqual(pool.doneCount(), 0)


    def test_pool_execution(self):
        pool = ThreadPool(4)

        result = {}
        for index in range(10):
            pool.addTask(self.numberer, index, result=result)

        pool.nonBlockingStart()
        pool.join()

        for index in range(10):
            self.assertTrue(index in result)
            self.assertTrue(result[index])

        self.assertFalse(pool.hasFailedTasks())



    def test_pool_unbound_fail(self):
        pool = ThreadPool(4)

        self.assertEqual(4, pool.poolSize())
        pool.addTask(ThreadPoolTest.numberer, 0, {})

        pool.nonBlockingStart()
        pool.join()

        self.assertTrue(pool.hasFailedTasks())


    def test_fill_pool(self):
        pool = ThreadPool(4)

        for index in range(10):
            pool.addTask(self.sleepTask, 2)

        pool.nonBlockingStart()
        time.sleep(0.5)
        self.assertEqual(pool.doneCount(), 0)
        self.assertEqual(pool.runningCount(), 4)

        pool.join()



    def test_task(self):
        def sleeping():
            time.sleep(1)

        task = Task(sleeping)

        self.assertFalse(task.hasStarted())
        self.assertFalse(task.isRunning())
        self.assertFalse(task.isDone())

        task.start()

        self.assertTrue(task.hasStarted())
        self.assertTrue(task.isRunning())

        task.join()

        self.assertFalse(task.isRunning())
        self.assertTrue(task.isDone())


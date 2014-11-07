import multiprocessing
from threading import Thread
import time
import traceback


class Task(Thread):
    def __init__(self, func, *args, **kwargs):
        super(Task, self).__init__()
        self.__func = func
        self.__args = args
        self.__kwargs = kwargs

        self.__started = False
        self.__done = False
        self.__failed = False
        self.__start_time = None

        self.__verbose = False

    def setVerbose(self, verbose):
        self.__verbose = verbose

    def run(self):
        self.__start_time = time.time()
        self.__started = True
        try:
            self.__func(*self.__args, **self.__kwargs)
        except Exception:
            self.__failed = True
            traceback.print_exc()
        finally:
            self.__done = True

        if self.__verbose:
            running_time = time.time() - self.__start_time
            print("Running time of task: %f" % running_time)

    def isDone(self):
        return self.__done

    def hasStarted(self):
        return self.__started or self.isAlive()

    def isRunning(self):
        return self.hasStarted() and not self.__done

    def hasFailed(self):
        return self.__failed

    def join(self, timeout=None):
        while not self.hasStarted() or self.isRunning():
            time.sleep(0.01)



class ThreadPool(object):

    def __init__(self, size=None, verbose=False):
        super(ThreadPool, self).__init__()

        if size is None:
            size = multiprocessing.cpu_count()

        self.__size = size
        self.__task_list = []
        self.__pool_finished = False
        self.__runner_thread = None
        self.__verbose = verbose
        self.__start_time = None


    def addTask(self, func, *args, **kwargs):
        if self.__start_time is None:
            task = Task(func, *args, **kwargs)
            # task.setVerbose(self.__verbose)
            self.__task_list.append(task)
        else:
            raise UserWarning("Can not add task after the pool has started!")

    def poolSize(self):
        return self.__size

    def taskCount(self):
        return len(self.__task_list)

    def __allTasksFinished(self):
        for task in self.__task_list:
            if not task.isDone():
                return False

        return True

    def runningCount(self):
        count = 0

        for task in self.__task_list:
            if task.isRunning():
                count += 1

        return count

    def doneCount(self):
        count = 0

        for task in self.__task_list:
            if task.isDone():
                count += 1

        return count

    def __findNextTask(self):
        for task in self.__task_list:
            if not task.hasStarted():
                return task

        return None

    def __start(self):
        while not self.__allTasksFinished():
            if self.runningCount() < self.poolSize():
                task = self.__findNextTask()

                if task is not None:
                    task.start()

            time.sleep(0.001)

        self.__pool_finished = True


    def nonBlockingStart(self):
        self.__runner_thread = Thread()
        self.__runner_thread.run = self.__start
        self.__runner_thread.start()
        self.__start_time = time.time()

    def join(self):
        while not self.__pool_finished:
            time.sleep(0.001)

        if self.__verbose:
            running_time = time.time() - self.__start_time
            print("Running time: %f using a pool size of: %d" % (running_time, self.poolSize()))

    def hasFailedTasks(self):
        for task in self.__task_list:
            if task.hasFailed():
                return True

        return False

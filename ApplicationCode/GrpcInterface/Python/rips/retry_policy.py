import abc

import time


class RetryPolicy(abc.ABC):
    @abc.abstractmethod
    def sleep(self, retry_num):
        """
        How long to sleep in milliseconds.
        :param retry_num: the number of retry (starting from zero)
        """
        assert retry_num >= 0

    @abc.abstractmethod
    def time_out_message(self):
        """
        Generate a error message for user on time out.
        """
        pass

    @abc.abstractmethod
    def num_retries(self):
        """
        Max number retries.
        """
        pass


class FixedRetryPolicy(RetryPolicy):
    def __init__(self, sleep_time=1000, max_num_retries=10):
        """
        Create a fixed time retry policy.
        :param sleep_time: time to sleep in milliseconds.
        :param max_num_retries: max number of retries.
        """
        self.sleep_time = sleep_time
        self.max_num_retries = max_num_retries

    def sleep(self, retry_num):
        time.sleep(self.sleep_time / 1000)

    def time_out_message(self):
        return "Tried {} times with {} milliseconds apart.".format(
            self.max_num_retries, self.sleep_time
        )

    def num_retries(self):
        return self.max_num_retries

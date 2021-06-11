import abc

import time
import random


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


class ExponentialBackoffRetryPolicy(RetryPolicy):
    def __init__(self, min_backoff=200, max_backoff=10000, max_num_retries=20):
        """
        Create a truncated exponential backoff policy.
        See: https://en.wikipedia.org/wiki/Exponential_backoff
        :param min_backoff: minimum time to sleep in milliseconds.
        :param max_backoff: maximum time to sleep in milliseconds.
        :param max_num_retries: max number of retries.
        """
        self.min_backoff = min_backoff
        self.max_backoff = max_backoff
        self.max_num_retries = max_num_retries
        self.multiplier = 2

    def sleep(self, retry_num):
        # Add a random component to avoid synchronized retries
        wiggle = random.randint(0, 100)
        sleep_ms = min(
            self.min_backoff + self.multiplier ** retry_num + wiggle, self.max_backoff
        )
        time.sleep(sleep_ms / 1000)

    def time_out_message(self):
        return (
            "Tried {} times with increasing delay (from {} to {} milliseconds).".format(
                self.max_num_retries, self.min_backoff, self.max_backoff
            )
        )

    def num_retries(self):
        return self.max_num_retries

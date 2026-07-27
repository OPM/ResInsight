import abc
import random
import time


class RetryPolicy(abc.ABC):
    @abc.abstractmethod
    def sleep(self, retry_num: int) -> None:
        """
        How long to sleep in milliseconds.
        :param retry_num: the number of retry (starting from zero)
        """
        assert retry_num >= 0

    @abc.abstractmethod
    def time_out_message(self) -> str:
        """
        Generate a error message for user on time out.
        """

    @abc.abstractmethod
    def num_retries(self) -> int:
        """
        Max number retries.
        """


class FixedRetryPolicy(RetryPolicy):
    def __init__(self, sleep_time: int = 1000, max_num_retries: int = 10):
        """
        Create a fixed time retry policy.
        :param sleep_time: time to sleep in milliseconds.
        :param max_num_retries: max number of retries.
        """
        self.sleep_time: int = sleep_time
        self.max_num_retries: int = max_num_retries

    def sleep(self, retry_num: int) -> None:
        time.sleep(self.sleep_time / 1000)

    def time_out_message(self) -> str:
        return f"Tried {self.max_num_retries} times with {self.sleep_time} milliseconds apart."

    def num_retries(self) -> int:
        return self.max_num_retries


class ExponentialBackoffRetryPolicy(RetryPolicy):
    def __init__(
        self,
        min_backoff: int = 200,
        max_backoff: int = 10000,
        max_num_retries: int = 20,
    ):
        """
        Create a truncated exponential backoff policy.
        See: https://en.wikipedia.org/wiki/Exponential_backoff
        :param min_backoff: minimum time to sleep in milliseconds.
        :param max_backoff: maximum time to sleep in milliseconds.
        :param max_num_retries: max number of retries.
        """
        self.min_backoff: int = min_backoff
        self.max_backoff: int = max_backoff
        self.max_num_retries: int = max_num_retries
        self.multiplier: int = 2

    def sleep(self, retry_num: int) -> None:
        # Add a random component to avoid synchronized retries
        wiggle = random.randint(0, 100)
        sleep_ms = min(
            self.min_backoff + self.multiplier**retry_num + wiggle, self.max_backoff
        )
        time.sleep(sleep_ms / 1000)

    def time_out_message(self) -> str:
        return (
            f"Tried {self.max_num_retries} times with increasing delay (from {self.min_backoff} to {self.max_backoff} milliseconds)."
        )

    def num_retries(self) -> int:
        return self.max_num_retries

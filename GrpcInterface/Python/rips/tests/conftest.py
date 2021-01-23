import pytest
import sys
import os
import getopt

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

_rips_instance = None


@pytest.fixture
def rips_instance():
    return _rips_instance


@pytest.fixture
def initialize_test():
    _rips_instance.project.close()  # make sure ResInsight is clean before execution of test
    yield initialize_test
    _rips_instance.project.close()  # make sure ResInsight is clean after test


def pytest_addoption(parser):
    parser.addoption(
        "--console",
        action="store_true",
        default=False,
        help="Run as console application",
    )
    parser.addoption(
        "--existing",
        action="store_true",
        default=False,
        help="Look for existing ResInsight",
    )


def pytest_configure(config):
    global _rips_instance
    console = False
    if config.getoption("--existing"):
        print("Looking for existing ResInsight")
        _rips_instance = rips.Instance.find()
    else:
        if config.getoption("--console"):
            console = True
        _rips_instance = rips.Instance.launch(console=console)
    if not _rips_instance:
        print("Need a valid ResInsight executable to launch tests")
        exit(0)


def pytest_unconfigure(config):
    if not config.getoption("--existing"):
        if _rips_instance:
            _rips_instance.exit()

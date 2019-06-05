import pytest
import sys
import os
import getopt

sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

_rips_instance = None

@pytest.fixture
def rips_instance():
    return _rips_instance

@pytest.fixture
def initializeTest():
    _rips_instance.project.close()

def pytest_addoption(parser):
    parser.addoption("--console", action="store_true", default=False, help="Run as console application")

def pytest_configure(config):
    global _rips_instance
    console = False
    if config.getoption('--console'):
        print("Should run as console app")
        console = True
    _rips_instance = rips.Instance.launch(console=console)

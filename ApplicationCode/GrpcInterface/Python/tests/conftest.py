import pytest
import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

instance = rips.Instance.launch()

@pytest.fixture
def rips_instance():
    return instance

def pytest_unconfigure(config):
    print("Telling ResInsight to Exit")
    instance.app.exit()
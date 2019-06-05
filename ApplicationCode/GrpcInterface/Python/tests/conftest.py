import pytest
import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

instance = rips.Instance.launch()

if (not instance):
    print("A ResInsight instance is required for running the tests")
    exit(1)

@pytest.fixture
def rips_instance():
    return instance

@pytest.fixture
def initializeTest():
    instance.project.close()

def pytest_unconfigure(config):
    print("Telling ResInsight to Exit")
    instance.app.exit()
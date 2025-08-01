import sys
import os
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips


def test_set_and_get_values(rips_instance, initialize_test):
    project = rips_instance.project
    key = "my-key"
    input_values = [float(x) for x in range(2000)]
    project.set_key_values(key, input_values)

    output_values = project.key_values(key)
    assert input_values == output_values


def test_get_non_existing_key(rips_instance, initialize_test):
    project = rips_instance.project
    key = "this-key-does-not-exist"

    with pytest.raises(rips.RipsError, match="No matching key found."):
        project.key_values(key)


def test_remove_non_existing_key(rips_instance, initialize_test):
    project = rips_instance.project
    key = "this-key-does-not-exist"

    with pytest.raises(rips.RipsError, match="No matching key found."):
        project.remove_key_values(key)


def test_set_and_remove_and_get_values(rips_instance, initialize_test):
    project = rips_instance.project
    key = "my-key"
    input_values = [float(x) for x in range(2000)]
    project.set_key_values(key, input_values)

    output_values = project.key_values(key)
    assert input_values == output_values

    project.remove_key_values(key)

    with pytest.raises(rips.RipsError, match="No matching key found."):
        project.key_values(key)

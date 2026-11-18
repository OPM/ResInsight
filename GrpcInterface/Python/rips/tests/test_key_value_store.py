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


def test_set_and_get_values_async(rips_instance, initialize_test):
    project = rips_instance.project
    key = "my-async-key"
    input_values = [float(x) for x in range(2000)]
    project.set_key_values(key, input_values)

    accumulated = []
    chunk_count = 0
    for chunk in project.key_values_async(key):
        accumulated.extend(chunk.values)
        chunk_count += 1
    assert chunk_count > 0
    assert input_values == accumulated


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


@pytest.mark.parametrize(
    "dtype, input_values",
    [
        ("float32", [0.5, -1.25, 3.0e38]),
        ("float64", [7319649.123, -1.0e-300, 1.7e308]),
        ("int32", [-(2**31), 0, 2**31 - 1]),
        ("int64", [-(2**63), 0, 2**63 - 1]),
        ("uint32", [0, 16777217, 2**32 - 1]),
        ("uint64", [0, 2**53 + 1, 2**64 - 1]),
    ],
)
def test_set_and_get_typed_values(rips_instance, initialize_test, dtype, input_values):
    project = rips_instance.project
    key = "my-" + dtype + "-key"
    project.set_key_values(key, input_values, dtype=dtype)

    output_values = project.key_values(key, dtype=dtype)
    if dtype == "float32":
        assert output_values == pytest.approx(input_values, rel=1e-7)
    else:
        assert output_values == input_values

    # Reading with another element type fails
    other_dtype = "float32" if dtype != "float32" else "float64"
    with pytest.raises(rips.RipsError, match="element type"):
        project.key_values(key, dtype=other_dtype)

    project.remove_key_values(key)


def test_set_typed_values_out_of_range(rips_instance, initialize_test):
    project = rips_instance.project

    # The server checks that the values fit the element type
    with pytest.raises(rips.RipsError, match="out of range for element type UINT32"):
        project.set_key_values("too-large-uint32", [0, 1, 2**32], dtype="uint32")

    with pytest.raises(rips.RipsError, match="out of range for element type INT32"):
        project.set_key_values("too-large-int32", [2**31], dtype="int32")

    with pytest.raises(rips.RipsError, match="out of range for element type INT32"):
        project.set_key_values("too-small-int32", [-(2**31) - 1], dtype="int32")


def test_set_typed_values_out_of_range_in_later_chunk(rips_instance, initialize_test):
    project = rips_instance.project
    key = "partly-out-of-range-uint32"

    # The first chunk is accepted, the out of range value is in the second chunk
    input_values = list(range(project.chunk_size)) + [2**32]
    with pytest.raises(rips.RipsError, match="out of range for element type UINT32"):
        project.set_key_values(key, input_values, dtype="uint32")

    # Nothing is stored when the upload fails
    with pytest.raises(rips.RipsError, match="No matching key found."):
        project.key_values(key, dtype="uint32")


def test_unsupported_dtype(rips_instance, initialize_test):
    project = rips_instance.project

    with pytest.raises(ValueError, match="Unsupported dtype"):
        project.set_key_values("int16-key", [1, 2, 3], dtype="int16")

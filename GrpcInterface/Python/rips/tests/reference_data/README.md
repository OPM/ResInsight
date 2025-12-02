# Reference Data for Completion Tests

This directory contains reference JSON files used for regression testing of the ResInsight Python gRPC API.

## completion_data_reference.json

Reference data for testing the `GetCompletionData` gRPC service method. Contains expected COMPDAT and WELSPECS table entries for a well with fixed completion parameters:

- **Well**: Imported from `wellpath_a.dev` (TEST10K dataset)
- **Start MD**: 2200.0m
- **End MD**: 2600.0m  
- **Interval Length**: 400m
- **Diameter**: 0.25m
- **Skin Factor**: 0.1
- **Grid**: TEST10K_FLT_LGR_NNC

### Test Coverage

The 400m perforation interval generates **6 COMPDAT entries** covering:
- Multiple grid cells: (39,54), (39,55), (39,56)
- Multiple K layers: 15-18  
- Various completion directions: Y, Z
- Different transmissibility and KH values

### Usage

The reference file is automatically used by `test_completion_data_regression_reference()` to validate that completion data generation remains consistent across code changes.

```python
# Test automatically compares current results against reference
pytest test_get_completion_data.py::test_completion_data_regression_reference
```

### Updating Reference Data

If intentional changes are made to completion data generation:

1. Delete the reference file: `rm completion_data_reference.json`
2. Run the test to regenerate: `pytest test_get_completion_data.py::test_completion_data_regression_reference`
3. Review the new reference data for correctness
4. Commit the new reference file to git

### Cross-Platform Compatibility

The tests are designed to produce identical results on Windows and Linux:

- **File Paths**: Uses `pathlib.Path` for cross-platform file paths
- **JSON Encoding**: UTF-8 encoding with `ensure_ascii=True` for consistent serialization
- **Field Ordering**: Alphabetical field ordering in dictionaries
- **Number Formatting**: Explicit `int()` and `float()` conversion with consistent rounding (6-8 decimal places)
- **JSON Serialization**: Fixed separators `(',', ':')` to eliminate whitespace variations
- **Hash Stability**: SHA256 with UTF-8 encoding for reproducible hashes across platforms
- **Data Types**: Explicit type casting to avoid platform-specific numeric representations

### Reference Data Structure

```json
{
  "compdat": [
    {
      "comment": "Perforation Completion: MD In: 2274.13 - MD Out: 2290.66 ...",
      "diameter": 0.25,
      "direction": "Z",
      "end_md": 2290.660636,
      "grid_i": 39,
      "grid_j": 54,
      "kh": 8.39888987,
      "lower_k": 16,
      "open_shut_flag": "OPEN",
      "skin_factor": 0.1,
      "start_md": 2274.130017,
      "transmissibility": 0.09835443,
      "upper_k": 16,
      "well_name": "ReferenceTestWell"
    },
    // ... 5 more COMPDAT entries
  ],
  "welspecs": [
    {
      "well_name": "ReferenceTestWell",
      "group_name": "1*",
      "grid_i": 29,
      "grid_j": 41,
      "phase": "OIL",
      // ... other WELSPECS fields
    }
  ]
}
```

## Maintenance

- Reference data should be updated whenever the completion data generation algorithm changes intentionally
- Failed regression tests indicate unexpected changes in completion data generation
- The test includes validation that modified data properly fails comparison (failure simulation)
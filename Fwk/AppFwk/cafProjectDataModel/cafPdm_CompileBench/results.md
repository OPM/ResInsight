# cafPdm_CompileBench — Compile Time Benchmark

## Configuration

| Setting | Value |
|---|---|
| Branch | `pdm-compile-bench` |
| Commit | `c7ec12fba` |
| Compiler | MSVC (cl.exe) |
| Build system | Ninja |
| Parallelism | 20 cores |
| buildcache | disabled |
| Precompiled headers | disabled |
| Unity build | disabled |
| Date | 2026-04-06 |

The benchmark target is an `OBJECT` library with `UNITY_BUILD OFF`, so each
translation unit is compiled in isolation. Times are wall-clock milliseconds
read from `.ninja_log` (`finish_time - start_time`).

## Raw Results (ms)

| File | Run 1 | Run 2 | Run 3 | **Avg** |
|---|---|---|---|---|
| bench_filepath_fields.cpp | 2352 | 2278 | 2370 | **2333** |
| bench_basic_fields.cpp | 2594 | 2544 | 2603 | **2580** |
| bench_container_fields.cpp | 2749 | 2686 | 2805 | **2747** |
| bench_enum_fields.cpp | 3835 | 3703 | 3785 | **3774** |
| bench_mixed_fields.cpp | 4497 | 4323 | 4554 | **4458** |

## Notes

- Each run was a full clean rebuild (`ninja -t clean cafPdm_CompileBench` followed by rebuild).
- The benchmark files exercise different combinations of PDM field types:
  - `bench_filepath_fields` — `caf::FilePath` fields
  - `bench_basic_fields` — primitive field types (`int`, `double`, `bool`, `QString`)
  - `bench_container_fields` — `std::vector` container fields
  - `bench_enum_fields` — enum fields with `AppEnum`
  - `bench_mixed_fields` — mix of all field types
- `bench_mixed_fields` is the heaviest translation unit (~4458 ms avg) and is the
  most representative proxy for a real PDM object with diverse field usage.

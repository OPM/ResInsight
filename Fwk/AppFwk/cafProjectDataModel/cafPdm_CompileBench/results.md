# cafPdm_CompileBench — Compile Time Benchmark

## Configuration

| Setting | Value |
|---|---|
| Compiler | MSVC (cl.exe) |
| Build system | Ninja |
| Parallelism | 2 cores |
| buildcache | disabled |
| Precompiled headers | disabled |
| Unity build | disabled |

The benchmark target is an `OBJECT` library with `UNITY_BUILD OFF`, so each
translation unit is compiled in isolation. Times are wall-clock milliseconds
read from `.ninja_log` (`finish_time - start_time`). Each run is a full clean
rebuild (`ninja -t clean cafPdm_CompileBench` followed by rebuild).


## Baseline — 2 cores, 3 runs — branch `pdm-compile-bench-01`, commit `e5577fda8` (2026-04-06)

| File | Run 1 | Run 2 | Run 3 | **Avg** | **Delta vs baseline (20 cores)** |
|---|---|---|---|---|---|
| bench_filepath_fields.cpp | 2239 | 2253 | 2258 | **2250** | -45 |
| bench_basic_fields.cpp | 2364 | 2399 | 2370 | **2378** | -142 |
| bench_container_fields.cpp | 2647 | 2669 | 2634 | **2650** | -123 |
| bench_enum_fields.cpp | 3389 | 3460 | 3380 | **3410** | -341 |
| bench_mixed_fields.cpp | 3938 | 4045 | 3978 | **3987** | -422 |

## After changes — 2 cores, 3 runs — branch `pdm-compile-bench-01`, commit `e5577fda8` + stash applied (2026-04-06)

| File | Run 1 | Run 2 | Run 3 | **Avg** | **Delta vs baseline (2 cores)** |
|---|---|---|---|---|---|
| bench_filepath_fields.cpp | 2229 | 2170 | 2158 | **2186** | -64 |
| bench_basic_fields.cpp | 2324 | 2342 | 2318 | **2328** | -50 |
| bench_container_fields.cpp | 2555 | 2601 | 2557 | **2571** | -79 |
| bench_enum_fields.cpp | 3284 | 3291 | 3316 | **3297** | -113 |
| bench_mixed_fields.cpp | 3886 | 3876 | 3898 | **3887** | -100 |

## Notes

- The benchmark files exercise different combinations of PDM field types:
  - `bench_filepath_fields` — `caf::FilePath` fields
  - `bench_basic_fields` — primitive field types (`int`, `double`, `bool`, `QString`)
  - `bench_container_fields` — `std::vector` container fields
  - `bench_enum_fields` — enum fields with `AppEnum`
  - `bench_mixed_fields` — mix of all field types
- `bench_mixed_fields` is the heaviest translation unit and is the most
  representative proxy for a real PDM object with diverse field usage.

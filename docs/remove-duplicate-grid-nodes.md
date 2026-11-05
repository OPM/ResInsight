# Removing duplicate grid nodes

`RigMainGrid::removeDuplicateNodes()` merges grid nodes with exactly equal coordinates. Grid readers store eight
separate nodes per cell, even when neighboring cells share corners, so merging them reduces memory use considerably.
The removal runs only when the experimental feature `remove-duplicate-grid-nodes` is enabled
(Preferences → System → Experimental Features).

The algorithm is implemented in `RigGridNodeDeduplication::removeDuplicateNodes()`
(`ApplicationLibCode/ReservoirDataModel/RigGridNodeDeduplication.cpp`). It works on a plain node vector and returns
the mapping from old to new node index. `RigMainGrid` uses the mapping to update the cell corner indices.

**Goal:** merge nodes whose coordinates are exactly equal, keep the first occurrence of each coordinate, keep the kept
nodes in their original order, and point cell corner indices at the new node table. The result is the same as a
simple sequential pass, whatever the thread count.

```mermaid
flowchart TD
    S([RigMainGrid::removeDuplicateNodes]) --> G{Already removed,<br/>empty, or ≥ 2³¹ nodes?}
    G -- yes --> R0([return 0])
    G -- no --> T["Allocate hash table<br/>capacity = power of 2 ≥ 1.5 × n<br/>fill with emptySlot (parallel)"]
    T --> P1["<b>Pass 1 – insert (parallel)</b><br/>probe from nodeHash(node) & mask<br/>empty slot → CAS in own index<br/>equal node → atomic min of index<br/>oldToNew[i] = slot"]
    P1 --> P2["<b>Pass 2 – resolve (parallel)</b><br/>first = table[slot]<br/>first == i → unique, count it for the block<br/>else oldToNew[i] = first | duplicateBit"]
    P2 --> F["Free table<br/>partial_sum of block counts → blockOffsets"]
    F --> D{removed > 0?}
    D -- no --> E
    D -- yes --> P3["<b>Pass 3 – number and copy unique nodes (parallel)</b><br/>new index starts at blockOffsets[block]<br/>newNodes[newIndex] = nodes[i]<br/>oldToNew[i] = newIndex++"]
    P3 --> P4["<b>Pass 4 – resolve duplicates (parallel)</b><br/>oldToNew[i] = oldToNew[first]"]
    P4 --> M["nodes = move(newNodes), return oldToNew<br/>RigMainGrid: remapCellCornerIndices(oldToNew)"]
    M --> E([mark removed, return removed count])
```

## Key ideas

| Idea | Why it matters |
|---|---|
| **Fixed blocks of 16,384 nodes** | Each block is numbered from its own offset, so new indices don't depend on how OpenMP schedules the work. |
| **Lock-free insertion** | Slots are claimed with `compare_exchange` through `std::atomic_ref`. A filled slot never changes coordinate; its index can only decrease, through an atomic min. After pass 1 each slot holds the **lowest** index for its coordinate, which is the first occurrence. |
| **`oldToNew` reused for every stage** | It holds the slot (pass 1), then the first index with a duplicate bit (pass 2), then the new index (passes 3–4). No extra per-node arrays are needed. |
| **Duplicate bit (bit 31)** | Separates "first index of my coordinate" from "my new index". This is why the node limit is 2³¹. |
| **Pass 4 has no races** | Duplicates only read entries of unique nodes, and those were finished in pass 3 and are never written in pass 4. |
| **`nodeHash`** | A splitmix64 mix of the exact bit pattern, with −0.0 normalized to 0.0. About 2.4x faster than `std::hash<double>` here. |
| **`isSameNode`** | Compares the same normalized bit patterns as `nodeHash`. It matches `operator==` except for NaN, so NaN nodes with equal bits are merged instead of each probing past all the earlier ones. |

`RigActiveCellGrid` overrides `remapCellCornerIndices()` so the corner indices of its own cell storage are remapped as
well.

## Example: two cells sharing a face

```mermaid
flowchart LR
    subgraph Before["Before: 16 nodes"]
        A["cell 0: n0…n7"]
        B["cell 1: n8…n15<br/>(n8,n10,n12,n14 equal n1,n3,n5,n7)"]
    end
    subgraph After["After: 12 nodes"]
        C["cell 0: 0…7"]
        D["cell 1: 1,8,3,9,5,10,7,11"]
    end
    Before -->|"4 duplicates merged"| After
```

## Cost

Measured with `RigMainGridImportPerformance.DISABLED_SyntheticGridRemoveDuplicateNodes` on a 350 x 350 x 80 synthetic
grid (9.8M cells, 78.4M → 10.0M nodes) with 32 threads:

- **Time:** 0.43–0.52 s, down from 1.82 s for the sequential version.
- **Temporary memory:** about +811 MiB, made up of the 512 MiB hash table and 300 MiB for `oldToNew`. The new node
  table (228 MiB) is allocated after the hash table is freed.
- **Retained memory:** the node table drops from about 1794 MiB to 228 MiB.

Run the performance tests with:

```
ResInsight-tests --gtest_also_run_disabled_tests --gtest_filter=RigMainGridImportPerformance.*
```

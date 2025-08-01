# Aether: Baseline Performance Benchmarks

## 1. System Specification
This document details the baseline performance of the core C++ matching engine. The following tests were conducted on the un-optimized, single-threaded engine to establish a performance baseline before any optimization work.

---

## 1. System Specifications

* **Hardware**:
    * **Chip**: Apple M2
    * **CPU**: 8-Core
    * **Memory**: 8gb
* **Software**:
    * **OS**: macOS Sequoia 15.5
    * **Compiler**: Apple clang version 17.0.0 (clang-1700.0.13.3)
    * **Build Type**: Release

## 2. Benchmark Methodology

The benchmarks were compiled in `Release` mode and executed using the Google Benchmark library.

To ensure accurate and isolated measurements for each operation, the order book state was reset for every iteration of the benchmark loop. Each test measures a single operation against a deep order book pre-populated with **2,000** resting orders (1,000 bids and 1,000 asks).

The following three distinct operations were measured:

* **`BM_RealisticTrade`**: Measures the time to process an aggressive sell order that results in a single partial-fill trade against a resting bid order at the best price.
* **`BM_PassiveInsert`**: Measures the time to add a new passive buy order to the book that does not cross the spread and results in no trade.
* **`BM_RemoveOrder`**: Measures the time to find and remove a resting order from the middle of the order book.

---

## 3. Baseline Results (Un-optimized)

The following results represent the average time per operation.

| Benchmark         | Average Time (ns) | CPU Time (ns) |
| ----------------- | ----------------: | -------------:|
| `BM_RealisticTrade` |         86,596 ns |     80,635 ns |
| `BM_PassiveInsert`  |         79,955 ns |     78,309 ns |
| `BM_RemoveOrder`    |         78,109 ns |     78,012 ns |

---

## 4. Initial Analysis

The un-optimized engine demonstrates a baseline latency of approximately **80-90 µs** for core operations on a significantly populated book. The results are consistent, with trade execution being the most expensive operation as expected.

These initial results meet the project's p99 latency goal of being under 100 µs. However, the calculated throughput (approx. 11,500 - 12,800 OPS) is below the target of 100,000 OPS. This indicates that further optimization is required, which will be the focus of the next phase.

# High-Performance C++ Order Book Matching Engine

A low-latency Limit Order Book implemented in modern C++20, capable of running **100+ million** operations per second.
This project simulates the core matching logic used by financial exchanges (NASDAQ, CME, Binance) and is engineered for extreme performance, cache locality, and predictable low-latency behavior.
I built this to explore real-world market microstructure, systems programming, and high-frequency trading.

## Basic Overview

An **order book** is the central structure used by financial exchanges to match buyers and sellers. It keeps track of the current buy orders (bids) and sell orders (asks) for a given asset, usually a stock or crypto.
- A **buy order** means someone wants to purchase at a certain price or lower.
- A **sell order** means someone wants to sell at a certain price or higher.
- Each order has a price and a quantity.
  
Orders are grouped by price into two lists:
- **Bids**: sorted from highest to lowest price (best offers to buy)
- **Asks**: sorted from lowest to highest price (best offers to sell)

The best bid and best ask define the **market price**: the point where buyers and sellers are closest to agreement.

When a new order arrives, the engine checks if it can be matched with an existing order on the opposite side:
- If yes, a **trade** occurs.
- If not, the new order stays in the book, waiting for a future match.

This process repeats continuously, updating the market in real time.

## Architecture

My design emphasizes contiguous storage, predictable memory access patterns, and minimal dynamic allocation. The core classes are:

### OrderPool

A preallocated contiguous block of `Order` objects, providing O(1) allocation and deallocation, eliminating heap fragmenration and improving cache locality.

### PriceLevel

Each price level contains a doubly linked queue of orders, which ensures price-time priority without extra node allocations. Removing or inserting at either end is O(1) and cache-efficient.

### OrderBookSide

A `boost::container::flat_map`that stores price levels in sorted contiguous memory. It provides logarithmic lookups with significantly better locality compared to a regular `std::map`, which uses red-black trees.

### OrderLookup

An `unordered_map` mapping OrderId -> (Side, Order*). This allows O(1) cancels and modifications.

### MatchingEngine

The coordinator. Handles.
- Matching new orders
- Maintaining book structure
- Tracking best prices
- Canceling, modifying, and allocating orders

It produces a predictable and efficient hot path, allowing operations to complete in only a few nanoseconds.

## Performance benchmarks

All benchmarks were compiled with `-O3` and measured using Google Benchmark on a 12-core 2.6 GHz machine.

![Benchmarks](/images/benchmark.png)

### Observations

- Core matching operations run in 8–10 nanoseconds, comparable to specialized low-latency engines.
- Best-price queries are ~2 nanoseconds, typically L1-cache hits.
- Modify/cancel-heavy workloads are slower (as expected) due to necessary shifting in flat_map.
- The engine sustains tens of millions of operations per second on common hardware.

## Motivation

Modern exchanges must process millions of events per second with minimal latency.
This project was built to study:
- Market microstructure
- Data structure design for low latency
- CPU caching behavior
- Memory access patterns
- High-performance C++ programming

The goal was to create a realistic, efficient, and extensible matching engine suitable for experimentation and learning.

## Running benchmarks

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/order_book_bench
```

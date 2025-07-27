# Aether: High-Performance Equity Matching Engine

## 1. System Overview

This project is a high-performance, low-latency equity order matching engine. It is designed to simulate the core functionality of a modern financial exchange, processing new orders, maintaining a live order book, and executing trades with microsecond-level performance. The system is architected for scalability and observability, providing a real-time view of market activity through a web-based interface. The primary goal is to process over 100,000 orders per second on a single node with p99 latency below 100 microseconds for order processing.

## 2. Component Diagram

```mermaid
graph TD
    subgraph Browser
        A[React Frontend]
    end

    subgraph API Layer
        B(Python API Server)
    end

    subgraph Core Logic
        C(C++ Matching Engine)
    end

    subgraph Persistence
        D[(PostgreSQL DB)]
    end

    A -- WebSocket (Live Data) --> B
    A -- REST/gRPC (Submit Orders) --> B
    B -- gRPC --> C
    C -- Async Write --> D
```

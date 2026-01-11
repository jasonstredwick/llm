### Policy-Driven LLM HTTP Execution Layer

## Overview

This library provides a policy-driven HTTP execution layer for large-scale LLM request workloads. It executes thousands of logical requests across multiple LLM endpoints while maintaining predictable behavior, bounded resource usage, and explicitly declared network policy.

The system intentionally separates:
- Transport mechanics (libcurl)
- Request intent
- Execution policy

## Core Goals

1. Support high fan-out LLM workloads across multiple endpoints.
2. Ensure all non-trivial behaviors (retry, jitter, smoothing) are policy-declared, not implicit.
3. Avoid hidden heuristics or transport-level surprises.
4. Respect unknown or partially known endpoint rate limits.
5. Protect local and remote resources.
6. Provide deterministic completion signaling.

## Fundamental Concepts

# Endpoint

An endpoint is identified by:
- scheme
- host
- port
- negotiated protocol

Each endpoint is treated as an independent policy domain unless configured otherwise.

## Request

A request represents one logical LLM call and includes:

- endpoint reference
- credentials (if any)
- payload
- completion handler
- optional policy overrides

Requests are independent, except where policy groups them.

## Policy (First-Class Concept)

A Policy is a declarative description of how requests may be executed.

Policies are:
- immutable once applied to a request
- evaluated by the client, not by libcurl
- explicit and observable

Policy controls may include:

# Retry Policy
- retry eligibility (by error class)
- maximum attempts
- backoff strategy
- retry budget (per request or per endpoint)

# Jitter Policy
- pre-submission delay range
- distribution (uniform, bounded random)
- whether jitter applies per request or per retry

# Network Smoothing Policy
- maximum in-flight requests
- pacing constraints
- burst limits
- optional credential-scoped throttling

# Timeout Policy
- connect timeout
- request timeout
- total deadline

Policies do not guarantee success — they only define allowed behavior.

## Rate Limiting Assumptions

The system explicitly acknowledges two possible endpoint rate-limiting models:

# Model A — Endpoint / IP based
- Rate limits apply regardless of credentials.
- libcurl connection limits suffice.
- Excess requests queue naturally.

# Model B — Credential based
- Rate limits apply per API key or credential.
- Requests sharing a credential must be coordinated.
- Requests with different credentials should not block each other.

Which model applies:
- may be unknown
- may differ by endpoint
- may change over time

Policy determines whether credential-scoped coordination is enabled.

## Backpressure & Streaming Semantics

The library treats libcurl write callbacks as the final consumption boundary for response data. libcurl does not
buffer full responses internally; data is delivered incrementally to the write callback as it is read from
the network.

To preserve correct end-to-end backpressure:
- Response data is consumed at the rate of the slowest downstream consumer.
- If downstream processing cannot keep up, the write callback pauses the transfer, causing libcurl to stop
  reading from the network and propagate backpressure to the server.
- No internal buffering is performed by default.

Decoupling network reads from downstream consumption (e.g., draining into internal storage and delivering later)
breaks protocol-level backpressure and is treated as an explicit, opt-in tradeoff that requires bounded buffering
and clear abort semantics.

## Threading Model

The library core is single-threaded. All transport operations are driven by a single owning thread per transport engine (CURLM). Parallelism is achieved by running multiple independent transport engines on separate threads when required. The library does not perform implicit multi-threading; users may invoke the library from any thread and, if desired, designate that thread as a coordinator responsible for managing additional worker threads.

## Execution Model
1. A request is submitted with an associated policy.
2. The client:
   - resolves endpoint
   - applies policy rules
   - optionally places the request into a policy-scoped queue
3. When policy allows:
   - the request is submitted to libcurl
4. libcurl:
   - manages connections
   - manages HTTP/2 multiplexing
   - applies protocol-level backpressure
5. On completion:
   - the request emits a completion signal
   - policy determines whether a retry is allowed

## Completion Signaling

Each request produces exactly one terminal completion signal, containing:
- final status
- disposition classification
- attempt count
- optional diagnostics

Completion signaling is:
- reliable
- non-duplicated
- policy-aware

## Resource Protection

The system enforces conservative global limits:
- total connections
- per-endpoint connections
- in-flight request caps
- bounded memory usage

libcurl is used strictly as a transport engine; it does not make policy decisions.

## Explicit Non-Goals

This system does not:

- infer undocumented rate limits
- dynamically tune concurrency heuristically
- retry implicitly
- expose protocol internals
- maximize throughput at the expense of determinism

## Design Philosophy

Transport is mechanical.
Policy is intentional.
Behavior must be explainable.

Every observable behavior in the system should trace back to:
- an explicit policy decision
- or a documented protocol constraint

Nothing else.

## Why This Matters

This approach ensures:
- no hidden retries
- no surprise connection creation
- no accidental thundering herds
- no debugging “ghost behavior”

Which is exactly what broke httpx for your use case.

## Implementation

# Security / TLS
- Endpoints may require explicit trust configuration.
- The library does not infer or weaken trust automatically.
- Trust must be declared per endpoint.

# Global Initialization
The library performs a single process-wide initialization of libcurl. This initialization is not restartable and must
occur before any client or request objects are created. Optional global memory allocator overrides may be provided at
initialization time. Teardown and reinitialization of libcurl within the same process are not supported. A full
reset of libcurl state requires a process restart. Client-level reset and resource cleanup are supported and
encouraged.

# Operations
The library models LLM invocations as operations with explicit completion semantics. Operations may consist of one
or more transport requests and may complete independently, as a group, or incrementally. Admission and scheduling
enforce endpoint-specific constraints before requests enter the transport layer.

# References

Please review the following documents for additional context:
- [LLM Client Design](llm_client_design.md)
- [LLM Operation Design](llm_operation_design.md)

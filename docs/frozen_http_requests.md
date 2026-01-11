### Frozen HTTP Request Boundary

## Motivation

During implementation of the HTTP execution layer (libcurl-based), it became clear that HTTP request construction
and HTTP request execution are fundamentally different concerns and should be separated explicitly.

libcurl operates strictly on bytes:
- it sends a stream of bytes as the request body
- it receives a stream of bytes as the response body
- it has no understanding of higher-level semantics such as JSON fields, prompts, images, file paths, or
  multipart structure

At the same time, higher-level LLM APIs express requests in rich semantic terms:
- system prompts
- user prompts
- images
- attachments
- structured JSON schemas
Conflating these layers would unnecessarily complicate retries, orchestration, and correctness.

## Design Principle

HTTP request construction is a pure transformation from semantic inputs to bytes.
HTTP request execution consumes only bytes.

Once a request enters the orchestration/execution layer, it is treated as immutable.

## Frozen HTTP Request

We introduce the concept of a Frozen HTTP Request, which represents a fully-materialized HTTP request ready
for execution.

A Frozen HTTP Request consists of:
- a complete set of HTTP headers
- a single, rewindable request body represented as a byte buffer (or equivalent byte stream descriptor)

Example conceptual structure:

struct FrozenHttpRequest {
    std::vector<HttpHeader> headers;
    std::vector<std::byte> body;
};


This object is:
- fully serialized
- transport-ready
- independent of any higher-level semantics

## Request Construction Boundary

All semantic processing occurs before a request becomes frozen. This includes:
- JSON serialization
- base64 encoding of binary data (e.g. images)
- multipart construction (if required)
- optional client-side conveniences (e.g. loading files from paths)

Once the request is frozen:
- the original semantic inputs may be discarded
- the orchestrator and retry logic operate exclusively on bytes
- retries resend identical byte streams, ensuring determinism

This establishes a clean boundary:

Domain Inputs
  (prompts, images, files)
        ↓
Request Builder
  (JSON / multipart serialization)
        ↓
Frozen HTTP Request
  (headers + byte buffer)
        ↓
HTTP Execution / Attempts
  (libcurl, retries, orchestration)

## Implications for Retries

Retries are modeled as new Attempts using the same Frozen HTTP Request:
- no re-serialization
- no mutation of request content
- no drift between attempts

This guarantees:
- correctness
- reproducibility
- clear retry semantics

The retry framework does not need to understand request structure or semantics.

## Simplification of the Execution Layer

By enforcing the frozen request boundary:
- the HTTP execution layer only deals with bytes
- libcurl integration remains simple and correct
- multipart vs JSON is irrelevant to execution
- future optimizations (e.g. pooling, zero-copy) remain possible without semantic changes

Each Attempt uploads exactly one request body byte stream, which accurately reflects the HTTP model
(multipart included).

## Convenience APIs

Client-side convenience features (e.g. specifying file paths instead of raw bytes) are explicitly
treated as request-builder concerns, not transport concerns.

These helpers:
- run before request freezing
- produce byte buffers
- never interact directly with libcurl

This keeps the core system:
- predictable
- testable
- free of filesystem assumptions

## Summary
- HTTP execution is byte-oriented; semantics live above it.
- Requests are frozen into immutable byte streams before orchestration.
- The orchestrator and retry logic operate only on bytes.
- This separation simplifies retries, correctness, and future evolution.
- This design intentionally trades early flexibility for clarity, determinism, and correctness, while keeping optimization paths open if needed later.

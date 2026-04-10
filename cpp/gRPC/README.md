# C++ gRPC Learning Project

This project contains teaching examples for gRPC C++:

1. Unary (simple mode)
2. Client streaming mode
3. Server streaming mode
4. Bidirectional streaming mode
5. Asynchronous full matrix with CompletionQueue state machines:
   - Async unary
   - Async client-stream
   - Async server-stream
   - Async bidirectional-stream
6. TLS/mTLS secure channel examples (cert generation + client validation)

It also demonstrates:

- Request metadata (`x-token`, `x-client-id`)
- Server initial/trailing metadata
- Deadline and cancellation checks via context
- Channel/server basic configuration (keepalive, message sizes)

## Files

- `proto/demo.proto`: service and message definitions
- `src/sync_server.cc`: sync server implementing unary + 3 streaming RPCs
- `src/sync_client.cc`: sync client invoking all four sync RPC modes
- `src/async_server.cc`: async server implementing all 4 async RPC modes with CallData state machines
- `src/async_client.cc`: async client implementing all 4 async RPC modes with CQ state machines
- `src/tls_server.cc`: TLS/mTLS server sample
- `src/tls_client.cc`: TLS/mTLS client sample with server cert validation
- `cmake/generate_certs.cmake`: teaching script for CA/server/client cert generation

## Build

Requirements:

- CMake >= 3.20
- Protobuf C++
- gRPC C++ (with CMake config packages)
- `grpc_cpp_plugin` in PATH
- `openssl` in PATH (for TLS/mTLS cert generation)

```bash
cmake -S . -B build
cmake --build build -j
```

Generate certificates only:

```bash
cmake --build build --target generate_certs
```

Certificates are generated into `build/certs`.

## Run - Insecure Examples

Terminal A (sync server):

```bash
./build/sync_server 0.0.0.0:50051
```

Terminal B (sync client):

```bash
./build/sync_client 127.0.0.1:50051
```

Terminal C (async server, 4 async RPC modes):

```bash
./build/async_server 0.0.0.0:50052
```

Terminal D (async client, 4 async RPC modes):

```bash
./build/async_client 127.0.0.1:50052
```

## Run - TLS/mTLS Examples

Start TLS server (server cert only):

```bash
./build/tls_server 0.0.0.0:50061 tls build/certs
```

TLS client validation demo (verify server certificate with CA):

```bash
./build/tls_client 127.0.0.1:50061 tls build/certs
```

Start mTLS server (require and verify client certificate):

```bash
./build/tls_server 0.0.0.0:50062 mtls build/certs
```

mTLS client demo (present client cert + verify server cert):

```bash
./build/tls_client 127.0.0.1:50062 mtls build/certs
```

## Auth Metadata Demo

The server expects metadata header `x-token=demo-token`.

- Missing token: `UNAUTHENTICATED`
- Wrong token: `PERMISSION_DENIED`
- Correct token: request accepted

## Suggested Exercises

1. Add per-RPC auth interceptor.
2. Add TLS certificate rotation strategy.
3. Add separate completion queues and worker threads for each async RPC type.

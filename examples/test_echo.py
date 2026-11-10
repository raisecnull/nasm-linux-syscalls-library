#!/usr/bin/env python3
"""Checks that echo_server (port 8080) echoes data back, incl. a large payload
and several clients in a row."""
import socket, sys

HOST, PORT = "127.0.0.1", 8080

def roundtrip(data: bytes) -> bytes:
    with socket.create_connection((HOST, PORT), timeout=3) as s:
        s.sendall(data)
        s.shutdown(socket.SHUT_WR)
        out = b""
        while True:
            chunk = s.recv(4096)
            if not chunk:
                return out
            out += chunk

tests = {
    "short message": b"hello asm\n",
    "binary bytes": bytes(range(256)),
    "large (100 KB)": b"A" * 100_000,
}
ok = True
for name, payload in tests.items():
    try:
        good = roundtrip(payload) == payload
    except OSError as e:
        print(f"FAIL  {name}: {e} (is ./echo_server running?)"); sys.exit(1)
    print(("PASS  " if good else "FAIL  ") + name)
    ok &= good
sys.exit(0 if ok else 1)

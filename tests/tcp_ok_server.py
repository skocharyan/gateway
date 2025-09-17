#!/usr/bin/env python3
"""
Simple TCP server for tests: accepts connections, reads incoming data,
and immediately responds with 'OK'.

Usage:
  python tests/tcp_ok_server.py [--host HOST] [--port PORT]

Defaults:
  HOST = 0.0.0.0
  PORT = 1500

This is intended to be used with the device's TCP client to validate
Ethernet::xTCPSendAndReceive behavior.
"""

import argparse
import socket
import sys
import threading

DEFAULT_HOST = "0.0.0.0"
DEFAULT_PORT = 7788
REPLY = b"OK"


def handle_client(conn, addr):
    try:
        print(f"Connection from {addr}")
        # Receive up to 4096 bytes (adjust as needed)
        data = conn.recv(4096)
        if not data:
            print(f"No data from {addr}")
            return
        print(f"Received {len(data)} bytes from {addr}: {data!r}")
        # Reply immediately with OK
        conn.sendall(REPLY)
        print(f"Sent response OK to {addr}")
    except Exception as e:
        print(f"Error handling client {addr}: {e}")
    finally:
        try:
            conn.shutdown(socket.SHUT_RDWR)
        except Exception:
            pass
        conn.close()


def run_server(host: str, port: int):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((host, port))
        srv.listen(5)
        print(f"TCP OK server listening on {host}:{port}")

        try:
            while True:
                conn, addr = srv.accept()
                # Handle each client in a new thread so multiple test runs can connect
                t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
                t.start()
        except KeyboardInterrupt:
            print("Server shutting down")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Simple TCP server that replies 'OK'")
    parser.add_argument("--host", default=DEFAULT_HOST, help="Host to bind (default 0.0.0.0)")
    parser.add_argument("--port", default=DEFAULT_PORT, type=int, help="Port to bind (default 1500)")
    args = parser.parse_args()

    try:
        run_server(args.host, args.port)
    except Exception as e:
        print(f"Fatal: {e}")
        sys.exit(1)

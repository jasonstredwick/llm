#!/usr/bin/env python3
"""
Start the mock LLM server in the background for integration tests.

Spawns mock_server.py, writes its PID to a file, and waits until
the /health endpoint responds before exiting.  CTest runs this as
a fixture SETUP step.

Usage:  python start_mock_server.py [port] [pid_file]
"""

import os
import subprocess
import sys
import time
import urllib.request
import urllib.error


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    pid_file = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), ".mock_server.pid"
    )

    script = os.path.join(os.path.dirname(os.path.abspath(__file__)), "mock_server.py")

    # Spawn the server as a detached subprocess.
    proc = subprocess.Popen(
        [sys.executable, script, str(port)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    # Write the PID so stop_mock_server.py can clean up.
    with open(pid_file, "w") as f:
        f.write(str(proc.pid))

    # Wait for the server to become ready (up to 5 seconds).
    url = f"http://127.0.0.1:{port}/health"
    deadline = time.monotonic() + 5.0
    while time.monotonic() < deadline:
        time.sleep(0.1)

        # Check the child hasn't already crashed.
        if proc.poll() is not None:
            print(f"[start_mock_server] Server exited prematurely (rc={proc.returncode})", file=sys.stderr)
            sys.exit(1)

        try:
            with urllib.request.urlopen(url, timeout=1) as resp:
                if resp.status == 200:
                    print(f"[start_mock_server] Server ready on port {port} (pid={proc.pid})")
                    sys.exit(0)
        except (urllib.error.URLError, OSError):
            pass

    print("[start_mock_server] Server did not become ready in time", file=sys.stderr)
    proc.terminate()
    sys.exit(1)


if __name__ == "__main__":
    main()

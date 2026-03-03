#!/usr/bin/env python3
"""
Stop the mock LLM server started by start_mock_server.py.

Reads the PID file, sends SIGTERM (or TerminateProcess on Windows),
waits briefly, then force-kills if needed.  CTest runs this as a
fixture CLEANUP step.

Usage:  python stop_mock_server.py [pid_file]
"""

import os
import signal
import sys
import time


def main():
    pid_file = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), ".mock_server.pid"
    )

    if not os.path.exists(pid_file):
        print("[stop_mock_server] No PID file found — nothing to do")
        return

    with open(pid_file) as f:
        pid = int(f.read().strip())

    os.remove(pid_file)

    # Try graceful termination first.
    try:
        if sys.platform == "win32":
            # Windows: no SIGTERM, use taskkill.
            os.system(f"taskkill /F /PID {pid} >nul 2>&1")
        else:
            os.kill(pid, signal.SIGTERM)
    except ProcessLookupError:
        print(f"[stop_mock_server] Process {pid} already exited")
        return

    # Wait up to 2 seconds for graceful exit.
    for _ in range(20):
        time.sleep(0.1)
        try:
            os.kill(pid, 0)  # Check if still alive (signal 0 = no-op).
        except ProcessLookupError:
            print(f"[stop_mock_server] Server (pid={pid}) stopped")
            return

    # Force kill.
    try:
        if sys.platform == "win32":
            os.system(f"taskkill /F /PID {pid} >nul 2>&1")
        else:
            os.kill(pid, signal.SIGKILL)
        print(f"[stop_mock_server] Server (pid={pid}) force-killed")
    except ProcessLookupError:
        pass


if __name__ == "__main__":
    main()

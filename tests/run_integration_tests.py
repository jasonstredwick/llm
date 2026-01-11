import subprocess
import time
import sys
import os
import signal

def run_integration_test(executable_path, mock_server_path):
    print(f"--- Starting Mock Server: {mock_server_path} ---")

    # Start the mock server in a new process
    server_proc = subprocess.Popen([sys.executable, mock_server_path],
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   text=True)

    # Give it a moment to bind to the port
    time.sleep(1)

    if server_proc.poll() is not None:
        print("Error: Mock server failed to start.")
        print(server_proc.stderr.read())
        return 1

    print(f"--- Running Test: {executable_path} ---")
    try:
        # Run the C++ test executable
        test_proc = subprocess.run([executable_path], capture_output=False)
        return_code = test_proc.returncode
    except Exception as e:
        print(f"Error executing test: {e}")
        return_code = 1
    finally:
        print("--- Shutting Down Mock Server ---")
        # Ensure server is killed
        if os.name == 'nt':
            subprocess.run(['taskkill', '/F', '/T', '/PID', str(server_proc.pid)],
                           capture_output=True)
        else:
            server_proc.terminate()
            server_proc.wait()

    return return_code

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python run_integration_tests.py <executable_path> <mock_server_path>")
        sys.exit(1)

    exe = sys.argv[1]
    server = sys.argv[2]

    if not os.path.exists(exe):
        print(f"Error: Executable not found at {exe}")
        sys.exit(1)

    res = run_integration_test(exe, server)
    sys.exit(res)

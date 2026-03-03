import http.server
import socketserver
import threading
import time
import sys
import json
from urllib.parse import urlparse, parse_qs

# Track request state for stateful test scenarios (thread-safe).
_request_count = {}  # path -> count
_request_lock = threading.Lock()


class MockLLMHandler(http.server.BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        # Suppress default logging to keep test output clean.
        pass

    def do_GET(self):
        if '/health' in self.path:
            response = {"status": "ok"}
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            return

        # Reset stateful counters
        if '/reset' in self.path:
            with _request_lock:
                _request_count.clear()
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"status": "reset"}')
            return

        self.send_response(404)
        self.end_headers()

    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode('utf-8')

        parsed_path = urlparse(self.path)
        params = parse_qs(parsed_path.query)
        delay_ms = int(params.get('delay_ms', [0])[0])

        if delay_ms > 0:
            time.sleep(delay_ms / 1000.0)

        # Simulate 429 Rate Limiting
        if 'simulate_429' in self.path:
            self.send_response(429)
            self.send_header('Retry-After', '1')
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"error": "Rate limit exceeded"}')
            return

        # Simulate 429 that succeeds on retry (stateful)
        if 'simulate_429_then_ok' in self.path:
            key = 'simulate_429_then_ok'
            should_429 = False
            with _request_lock:
                _request_count[key] = _request_count.get(key, 0) + 1
                if _request_count[key] <= 1:
                    should_429 = True
                else:
                    _request_count[key] = 0  # reset for next test
            if should_429:
                self.send_response(429)
                self.send_header('Retry-After', '0')
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(b'{"error": "Rate limit exceeded"}')
                return
            # Fall through — return OpenAI responses format on retry
            response = {
                "id": "resp_retry_ok",
                "object": "response",
                "created_at": int(time.time()),
                "status": "completed",
                "output": [{
                    "type": "message",
                    "role": "assistant",
                    "content": [{"type": "output_text", "text": "Retry succeeded"}]
                }],
                "usage": {"input_tokens": 5, "output_tokens": 3, "total_tokens": 8}
            }
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            return

        # Simulate 500 Internal Server Error
        if 'simulate_500' in self.path:
            self.send_response(500)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"error": {"message": "Internal server error"}}')
            return

        # ---- OpenAI Chat Completions (legacy, for test_curl compat) ----
        if '/v1/chat/completions' in self.path:
            req_data = json.loads(body) if body else {}
            model = req_data.get('model', 'unknown')

            response = {
                "id": "chatcmpl-123",
                "object": "chat.completion",
                "created": int(time.time()),
                "model": model,
                "choices": [
                    {
                        "index": 0,
                        "message": {
                            "role": "assistant",
                            "content": f"Response from {model}"
                        },
                        "finish_reason": "stop"
                    }
                ],
                "usage": {
                    "prompt_tokens": 10,
                    "completion_tokens": 20,
                    "total_tokens": 30
                }
            }

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            return

        # ---- OpenAI Responses API ----
        if '/v1/responses' in self.path:
            req_data = json.loads(body) if body else {}
            model = req_data.get('model', 'unknown')

            response = {
                "id": "resp_mock_001",
                "object": "response",
                "created_at": int(time.time()),
                "status": "completed",
                "model": model,
                "output": [
                    {
                        "type": "message",
                        "role": "assistant",
                        "content": [
                            {
                                "type": "output_text",
                                "text": f"Mock response from {model}"
                            }
                        ]
                    }
                ],
                "usage": {
                    "input_tokens": 12,
                    "output_tokens": 8,
                    "total_tokens": 20
                }
            }

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            return

        # ---- Anthropic Messages API ----
        if '/v1/messages' in self.path:
            req_data = json.loads(body) if body else {}
            model = req_data.get('model', 'unknown')

            response = {
                "id": "msg_mock_001",
                "type": "message",
                "role": "assistant",
                "model": model,
                "content": [
                    {
                        "type": "text",
                        "text": f"Mock response from {model}"
                    }
                ],
                "stop_reason": "end_turn",
                "stop_sequence": None,
                "usage": {
                    "input_tokens": 15,
                    "output_tokens": 10,
                    "cache_creation_input_tokens": 0,
                    "cache_read_input_tokens": 0
                }
            }

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            return

        # ---- Gemini generateContent API ----
        if 'generateContent' in self.path:
            req_data = json.loads(body) if body else {}

            # Extract model name from URL path:
            # /v1beta/models/{model}:generateContent?key=...
            model = "unknown"
            path_parts = parsed_path.path.split('/')
            for part in path_parts:
                if ':generateContent' in part:
                    model = part.replace(':generateContent', '')
                    break

            response = {
                "candidates": [
                    {
                        "content": {
                            "parts": [
                                {
                                    "text": f"Mock response from {model}"
                                }
                            ],
                            "role": "model"
                        },
                        "finishReason": "STOP",
                        "avgLogprobs": -0.5
                    }
                ],
                "usageMetadata": {
                    "promptTokenCount": 18,
                    "candidatesTokenCount": 12,
                    "totalTokenCount": 30
                },
                "modelVersion": model,
                "responseId": "gemini_mock_001"
            }

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            return

        self.send_response(404)
        self.end_headers()

class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    """Handle each request in a new thread so concurrent connections don't block."""
    allow_reuse_address = True
    daemon_threads = True


if __name__ == "__main__":
    PORT = 8080
    if len(sys.argv) > 1:
        PORT = int(sys.argv[1])

    with ThreadedTCPServer(("", PORT), MockLLMHandler) as httpd:
        print(f"Serving Mock LLM at port {PORT}")
        httpd.serve_forever()

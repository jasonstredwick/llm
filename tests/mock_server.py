import http.server
import socketserver
import time
import sys
import json
from urllib.parse import urlparse, parse_qs

class MockLLMHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if '/health' in self.path:
            response = {"status": "ok"}
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
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
            print(f"Delaying response by {delay_ms}ms")
            time.sleep(delay_ms / 1000.0)

        # Simulate 429 Rate Limiting
        if 'simulate_429' in self.path:
            self.send_response(429)
            self.send_header('Retry-After', '1')
            self.end_headers()
            self.wfile.write(b'{"error": "Rate limit exceeded"}')
            return

        # Simulate OpenAI Chat Completions
        if '/v1/chat/completions' in self.path:
            req_data = json.loads(body)
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

        self.send_response(404)
        self.end_headers()

if __name__ == "__main__":
    PORT = 8080
    if len(sys.argv) > 1:
        PORT = int(sys.argv[1])

    with socketserver.TCPServer(("", PORT), MockLLMHandler) as httpd:
        print(f"Serving Mock LLM at port {PORT}")
        httpd.serve_forever()

from http.server import BaseHTTPRequestHandler, HTTPServer
import json


class RequestHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        self.send_response(200)
        self.end_headers()
    
    def do_POST(self):
        content_length = int(self.headers.get("Content-Length", 0))

        body = self.rfile.read(content_length)

        print("\nReceived packet:")

        try:
            data = json.loads(body.decode("utf-8"))

            print(json.dumps(
                data,
                ensure_ascii=False,
                indent=2
            ))

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()

            self.wfile.write(
                b'{"status":"ok"}'
            )

        except Exception as error:
            print("Invalid JSON:", error)

            self.send_response(400)
            self.end_headers()


server = HTTPServer(
    ("localhost", 8080),
    RequestHandler
)

print("Server started on http://localhost:8080")
print("Waiting for POST requests...")

server.serve_forever()
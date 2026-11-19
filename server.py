#!/usr/bin/env python3
"""
Task Scheduling Optimization - Web API & Application Server
Serves the modern Web UI and provides REST API bridges to the native C engine.
"""

import os
import sys
import json
import subprocess
from http.server import HTTPServer, SimpleHTTPRequestHandler
import urllib.parse

PORT = 9090
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.join(BASE_DIR, "TaskSchedulingOptimization")
JOBS_FILE = os.path.join(PROJECT_DIR, "data", "jobs.txt")
C_EXECUTABLE = os.path.join(PROJECT_DIR, "task_scheduler")

def parse_jobs_txt():
    jobs = []
    if not os.path.exists(JOBS_FILE):
        return jobs
    with open(JOBS_FILE, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            if len(parts) >= 4:
                try:
                    jobs.append({
                        "id": int(parts[0]),
                        "name": parts[1],
                        "deadline": int(parts[2]),
                        "profit": int(parts[3])
                    })
                except ValueError:
                    continue
    return jobs

def save_jobs_txt(jobs):
    os.makedirs(os.path.dirname(JOBS_FILE), exist_ok=True)
    with open(JOBS_FILE, "w") as f:
        f.write("# Task Scheduling Optimization Dataset\n")
        f.write("# Format: id name deadline profit\n")
        for j in jobs:
            f.write(f"{j['id']} {j['name']} {j['deadline']} {j['profit']}\n")

class AppRequestHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=BASE_DIR, **kwargs)

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        if parsed.path == "/api/jobs":
            jobs = parse_jobs_txt()
            self.send_json_response(200, {"status": "ok", "jobs": jobs, "count": len(jobs)})
            return
        elif parsed.path == "/api/status":
            c_built = os.path.exists(C_EXECUTABLE) and os.access(C_EXECUTABLE, os.X_OK)
            self.send_json_response(200, {
                "status": "online",
                "c_binary_ready": c_built,
                "project_dir": PROJECT_DIR,
                "jobs_file_exists": os.path.exists(JOBS_FILE)
            })
            return
        elif parsed.path == "/":
            self.path = "/index.html"
            return super().do_GET()
        else:
            return super().do_GET()

    def do_POST(self):
        parsed = urllib.parse.urlparse(self.path)
        content_len = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_len).decode("utf-8") if content_len > 0 else "{}"
        
        try:
            data = json.loads(body) if body else {}
        except Exception:
            data = {}

        if parsed.path == "/api/save-jobs":
            jobs = data.get("jobs", [])
            save_jobs_txt(jobs)
            self.send_json_response(200, {"status": "ok", "message": f"Saved {len(jobs)} jobs to data/jobs.txt"})
            return

        elif parsed.path == "/api/build-c":
            # Recompile with make
            try:
                proc = subprocess.run(["make", "clean", "all"], cwd=PROJECT_DIR, capture_output=True, text=True, timeout=10)
                self.send_json_response(200, {
                    "status": "ok" if proc.returncode == 0 else "error",
                    "stdout": proc.stdout,
                    "stderr": proc.stderr,
                    "returncode": proc.returncode
                })
            except Exception as e:
                self.send_json_response(500, {"status": "error", "message": str(e)})
            return

        elif parsed.path == "/api/run-c":
            # Run C program with input simulation
            options = data.get("inputs", "1\n7\n8\n0\n")
            if not os.path.exists(C_EXECUTABLE):
                # Try building first
                subprocess.run(["make"], cwd=PROJECT_DIR, capture_output=True)
            
            try:
                proc = subprocess.run(
                    [C_EXECUTABLE],
                    cwd=PROJECT_DIR,
                    input=options,
                    capture_output=True,
                    text=True,
                    timeout=8
                )
                self.send_json_response(200, {
                    "status": "ok",
                    "stdout": proc.stdout,
                    "stderr": proc.stderr,
                    "returncode": proc.returncode
                })
            except Exception as e:
                self.send_json_response(500, {"status": "error", "message": str(e)})
            return

        self.send_json_response(404, {"status": "error", "message": "Endpoint not found"})

    def send_json_response(self, code, payload):
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()
        self.wfile.write(json.dumps(payload, indent=2).encode("utf-8"))

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

if __name__ == "__main__":
    server_address = ("", PORT)
    httpd = HTTPServer(server_address, AppRequestHandler)
    print(f"Task Scheduler Web UI Server running at http://localhost:{PORT}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server...")
        httpd.server_close()

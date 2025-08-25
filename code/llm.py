#!/usr/bin/env python3
import sys
import re
import time
import requests
import json
import argparse
import traceback
from typing import List, Optional
from concurrent.futures import ThreadPoolExecutor, as_completed

CHOICES = ["A", "B", "C", "D"]
ANSWER_GRAMMAR = re.compile(r"\b([ABCD])\b")

SYSTEM_PROMPT = (
    "Answer multiple-choice questions. You may explain, "
    "but your final line must contain only the correct option letter (A, B, C, or D)."
)

FEW_SHOT = [
    {"role": "user", "content": "Capital of France?\nA. London\nB. Berlin\nC. Paris\nD. Madrid"},
    {"role": "assistant", "content": "Paris is the capital of France.\nC"},
    {"role": "user", "content": "Atomic number 1?\nA. He\nB. O\nC. H\nD. C"},
    {"role": "assistant", "content": "Hydrogen has atomic number 1.\nC"},
]

def split_questions(raw: str) -> List[str]:
    parts = [p.strip() for p in re.split(r"\n\s*\n", raw) if p.strip()]
    return parts

def extract_think_content(text: str) -> str:
    match = re.search(r"<think>(.*?)</think>", text, re.DOTALL)
    if match:
        return match.group(1).strip()
    return text.strip()

def extract_choice(text: str) -> Optional[str]:
    lines = [line.strip() for line in text.strip().splitlines() if line.strip()]
    if lines:
        last_line = lines[-1]
        if last_line in CHOICES:
            return last_line
    cand = ANSWER_GRAMMAR.findall(text[-16:])
    if cand:
        return cand[-1]
    cand = ANSWER_GRAMMAR.findall(text)
    if cand:
        return cand[-1]
    return None

class LlamaTCPsolver:
    def __init__(self, host: str = "127.0.0.1", port: int = 8080, debug: bool = False):
        self.host = host
        self.port = port
        self.url = f"http://{host}:{port}/v1/chat/completions"
        self.debug = debug
        if self.debug:
            print(f"[DEBUG] LlamaTCPsolver initialized for server at {self.url}")

    def solve(self, question: str) -> str:
        messages = [{"role": "system", "content": SYSTEM_PROMPT}]
        messages.extend(FEW_SHOT)
        messages.append({"role": "user", "content": question })

        payload = {
            "model": "qwen2.5-7b",
            "messages": messages,
            "max_tokens": 1024,
            "temperature": 0.0,
        }

        if self.debug:
            print(f"[DEBUG] Sending payload:\n{json.dumps(payload, indent=2, ensure_ascii=False)}")

        max_retries = 1
        attempt = 0
        while attempt < max_retries:
            attempt += 1
            try:
                resp = requests.post(
                    self.url,
                    headers={"Content-Type": "application/json"},
                    data=json.dumps(payload),
                    timeout=120,
                )
                
                if self.debug:
                    print(f"[DEBUG] Received response status: {resp.status_code}")
                    print(f"[DEBUG] Received raw response body:\n{resp.text}")

                resp.raise_for_status()
                data = resp.json()

                if self.debug:
                    print(f"[DEBUG] Parsed response JSON:\n{json.dumps(data, indent=2, ensure_ascii=False)}")

                text = data["choices"][0]["message"]["content"]
                choice = extract_choice(text)

                if choice:
                    return choice
            except requests.exceptions.ConnectionError as e:
                if self.debug:
                    print(f"[DEBUG] Connection Error: Could not connect to {self.url}. Ensure llamafile server is running.", file=sys.stderr)
                break
            except requests.exceptions.HTTPError as e:
                if self.debug:
                    print(f"[DEBUG] HTTP Error: {e.response.status_code} - {e.response.reason}", file=sys.stderr)
                    print(f"[DEBUG] Response body: {e.response.text}", file=sys.stderr)
                break
            except Exception as e:
                if self.debug:
                    print(f"[DEBUG] An unexpected exception occurred: {e}", file=sys.stderr)
                    traceback.print_exc(file=sys.stderr)
                time.sleep(0.5)
                continue

        if self.debug:
            print(f"[DEBUG] Failed to get a valid response after {max_retries} attempts.", file=sys.stderr)
        return "E"
    def __init__(self, host: str = "127.0.0.1", port: int = 8080, debug: bool = False):
        self.host = host
        self.port = port
        self.url = f"http://{host}:{port}/v1/chat/completions"
        self.debug = debug
        if self.debug:
            print(f"[DEBUG] LlamaTCPsolver initialized for server at {self.url}")

    def solve(self, question: str) -> str:
        if self.debug:
            print(f"[DEBUG] Solving question:\n{question}\n")

        messages = [{"role": "system", "content": SYSTEM_PROMPT}]
        messages.extend(FEW_SHOT)
        messages.append({"role": "user", "content": question })

        payload = {
            "model": "qwen2.5-7b-instruct",
            "messages": messages,
            "max_tokens": 1024,
            "temperature": 0.0,
        }

        if self.debug:
            print(f"[DEBUG] Payload: {json.dumps(payload, ensure_ascii=False)}")

        max_retries = 1
        attempt = 0
        while attempt < max_retries:
            attempt += 1
            try:
                if self.debug:
                    print(f"[DEBUG] Sending request attempt {attempt} of {max_retries} for question: {question}")
                resp = requests.post(
                    self.url,
                    headers={"Content-Type": "application/json"},
                    data=json.dumps(payload),
                    timeout=120,
                )
                resp.raise_for_status()
                data = resp.json()
                if self.debug:
                    print(f"[DEBUG] Full response JSON: {json.dumps(data, ensure_ascii=False)}")
                text = data["choices"][0]["message"]["content"]
                choice = extract_choice(text)
                if self.debug:
                    print(f"[DEBUG] Received raw: '{text}', extracted choice: {choice}")
                if choice:
                    return choice
            except requests.exceptions.ConnectionError:
                if self.debug:
                    print(f"[DEBUG] Connection Error: Could not connect to {self.url}. Ensure llamafile server is running.")
                break
            except Exception as e:
                if self.debug:
                    print(f"[DEBUG] Exception on attempt {attempt} for {question}: {e}")
                    traceback.print_exc()
                time.sleep(0.5)
                continue

        if self.debug:
            print(f"[DEBUG] Failed to get a valid response after {max_retries} attempts for question: {question}")
        return "E"
def main():
    parser = argparse.ArgumentParser(description="Solve multiple-choice questions via a running Llamafile server")
    parser.add_argument("--host", default="127.0.0.1", help="Llamafile server host (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=8090, help="Llamafile server port (default: 8090)")
    parser.add_argument("--debug", action="store_true", help="Enable debug logs")
    parser.add_argument("--workers", type=int, default=4, help="Number of concurrent worker threads")
    args = parser.parse_args()

    raw = sys.stdin.read()
    questions = split_questions(raw)

    if not questions:
        print("[!] No questions found in standard input. Exiting.")
        return

    if args.debug:
        print(f"[DEBUG] Loaded {len(questions)} questions from stdin.")

    solver = LlamaTCPsolver(args.host, args.port, debug=args.debug)
    results = [None] * len(questions)  # 预分配结果列表，按原始顺序存储

    def process_question(idx: int, q: str):
        truncated_q = q.split('\n')[0] # Truncate for cleaner logs
        try:
            if args.debug:
                print(f"[DEBUG] Q{idx+1}: Starting to solve: '{truncated_q}'")
            ans = solver.solve(q)
            results[idx] = ans
            if args.debug:
                print(f"[DEBUG] Q{idx+1}: Finished. Answer: {ans}")
        except Exception as e:
            results[idx] = "E"
            if args.debug:
                print(f"[DEBUG] Q{idx+1}: An error occurred while processing: {e}", file=sys.stderr)

    # 提交任务（保留原始顺序索引）
    with ThreadPoolExecutor(max_workers=args.workers) as executor:
        futures = [
            executor.submit(process_question, idx, q)
            for idx, q in enumerate(questions)
        ]
        # 阻塞等待所有任务完成
        for future in as_completed(futures):
            future.result()  # 仅等待，不处理结果（已通过 results 列表存储）

    # 按原始顺序输出
    for ans in results:
        print(ans, flush=True)

if __name__ == "__main__":
    main()
from __future__ import annotations

import hashlib
import json
import subprocess
from pathlib import Path
from typing import Any
from urllib.parse import quote

from mkdocs.config.defaults import MkDocsConfig
from mkdocs.structure.files import Files
from mkdocs.structure.pages import Page


ROOT = Path(__file__).resolve().parent.parent
SCRIPT_PATH = ROOT / "scripts" / "compute_aigc_meta.mjs"
TARGET_SEGMENTS = ("posts/", "/posts/")
RAW_ROOT = "https://raw.githubusercontent.com/liulifox233/liulifox233.github.io/main/docs/"

_AIGC_RESULTS: dict[str, dict[str, Any] | None] = {}
_AIGC_CACHE_KEY = ""


def on_files(files: Files, config: MkDocsConfig, **kwargs: Any) -> Files:
    global _AIGC_RESULTS, _AIGC_CACHE_KEY

    jobs: dict[str, str] = {}
    digest = hashlib.sha256()

    for file in files.documentation_pages():
        src_uri = file.src_uri.replace("\\", "/")
        if not _should_process(src_uri):
            continue

        src_path = ROOT / "docs" / file.src_path
        if not src_path.exists():
            continue

        raw = src_path.read_text(encoding="utf-8")
        jobs[src_uri] = raw

        digest.update(src_uri.encode("utf-8"))
        digest.update(b"\0")
        digest.update(raw.encode("utf-8"))
        digest.update(b"\0")

    cache_key = digest.hexdigest()
    if cache_key == _AIGC_CACHE_KEY:
        return files

    _AIGC_RESULTS = _compute_rates(jobs)
    _AIGC_CACHE_KEY = cache_key
    return files


def on_page_markdown(markdown: str, page: Page, config: MkDocsConfig, files: Files, **kwargs: Any) -> str:
    result = _lookup_result(page)
    if not result:
        return markdown

    page.meta["ai_rate"] = result["ai_rate"]
    page.meta["aigc_verdict"] = result["verdict"]
    page.meta["aigc_char_rate"] = result["char_rate"]
    page.meta["aigc_sentence_rate"] = result["sentence_rate"]
    raw_url = _build_raw_url(page)
    if raw_url:
        page.meta["aigc_detection_url"] = f"/tools/aigc-detection/?url={quote(raw_url, safe='')}"
    return markdown


def _should_process(src_uri: str) -> bool:
    return src_uri.endswith(".md") and any(
        src_uri == segment[:-1] or src_uri.startswith(segment) or segment in src_uri
        for segment in TARGET_SEGMENTS
    )


def _lookup_result(page: Page) -> dict[str, Any] | None:
    candidates = []
    for value in (page.file.src_uri, page.file.src_path):
        normalized = value.replace("\\", "/")
        candidates.append(normalized)
        candidates.append(Path(normalized).name)

    for candidate in candidates:
        if not candidate:
            continue
        if candidate in _AIGC_RESULTS:
            return _AIGC_RESULTS[candidate]

    for candidate in candidates:
        if not candidate:
            continue
        for key, result in _AIGC_RESULTS.items():
            if key.endswith(f"/{candidate}") or key == candidate:
                return result

    return None


def _compute_rates(jobs: dict[str, str]) -> dict[str, dict[str, Any] | None]:
    if not jobs:
        return {}

    completed = subprocess.run(
        ["node", str(SCRIPT_PATH)],
        cwd=ROOT,
        input=json.dumps(jobs),
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            "AIGC metadata build step failed.\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}"
        )

    data = json.loads(completed.stdout)
    return {str(key): value for key, value in data.items()}


def _build_raw_url(page: Page) -> str | None:
    src_uri = page.file.src_uri.replace("\\", "/")
    if not _should_process(src_uri):
        return None
    return f"{RAW_ROOT}{src_uri}"

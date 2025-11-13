#!/usr/bin/env python3
"""
Utility script to translate Korean text inside C++ source/header files to English.

Features
--------
- Recursively scans the target directory for files with specified extensions (default: .cpp, .h).
- Detects contiguous Korean text segments and translates them to English using googletrans.
- Caches translations so identical phrases are translated once per run.
- Supports dry-run mode to preview replacements without modifying files.

Requirements
------------
pip install googletrans==4.0.0-rc1
"""

from __future__ import annotations

import argparse
import logging
import sys
from pathlib import Path
from typing import Dict, Iterable
import re

try:
    from googletrans import Translator  # type: ignore
except ImportError:  # pragma: no cover - handled at runtime
    Translator = None  # type: ignore[misc]


KOREAN_SEGMENT_PATTERN = re.compile(
    r"([\u3131-\u318E\uAC00-\uD7A3]+[^\u0000-\u007F]*)"
)


def iter_source_files(
    root: Path, extensions: Iterable[str]
) -> Iterable[Path]:
    for path in root.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix.lower() in extensions:
            yield path


def ensure_translator() -> "Translator":
    if Translator is None:
        raise RuntimeError(
            "googletrans is required. Install with 'pip install googletrans==4.0.0-rc1'."
        )
    return Translator()


def translate_segments(
    text: str, translator: "Translator", cache: Dict[str, str]
) -> str:
    def replace(match: re.Match[str]) -> str:
        segment = match.group(0)
        key = segment.strip()
        if not key:
            return segment
        if key not in cache:
            logging.debug("Translating segment: %s", key)
            try:
                translation = translator.translate(key, src="ko", dest="en")
                cache[key] = translation.text
            except Exception as exc:  # pragma: no cover - external dependency
                logging.warning("Failed to translate '%s': %s", key, exc)
                cache[key] = key  # fall back to original
        translated = cache[key]
        # Preserve leading/trailing whitespace around the segment
        prefix_len = len(segment) - len(segment.lstrip())
        suffix_len = len(segment) - len(segment.rstrip())
        return f"{segment[:prefix_len]}{translated}{segment[len(segment)-suffix_len:]}"

    return KOREAN_SEGMENT_PATTERN.sub(replace, text)


def process_file(
    path: Path,
    translator: "Translator",
    cache: Dict[str, str],
    dry_run: bool = False,
) -> bool:
    original_text = path.read_text(encoding="utf-8")
    if not KOREAN_SEGMENT_PATTERN.search(original_text):
        return False

    translated_text = translate_segments(original_text, translator, cache)
    if translated_text == original_text:
        return False

    if dry_run:
        logging.info("[DRY-RUN] Would update %s", path)
    else:
        path.write_text(translated_text, encoding="utf-8")
        logging.info("Updated %s", path)
    return True


def parse_args(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Translate Korean text in C++ source/header files to English."
    )
    parser.add_argument(
        "-r",
        "--root",
        type=Path,
        default=Path.cwd(),
        help="Root directory to scan (default: current working directory).",
    )
    parser.add_argument(
        "-e",
        "--extensions",
        nargs="*",
        default=[".cpp", ".h"],
        help="File extensions to include (default: .cpp .h).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show files that would change without writing modifications.",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Increase verbosity (-v, -vv).",
    )
    return parser.parse_args(list(argv))


def setup_logging(verbosity: int) -> None:
    level = logging.WARNING
    if verbosity == 1:
        level = logging.INFO
    elif verbosity >= 2:
        level = logging.DEBUG
    logging.basicConfig(
        level=level,
        format="%(levelname)s: %(message)s",
    )


def main(argv: Iterable[str]) -> int:
    args = parse_args(argv)
    setup_logging(args.verbose)

    root = args.root.resolve()
    if not root.exists():
        logging.error("Root directory does not exist: %s", root)
        return 1

    try:
        translator = ensure_translator()
    except RuntimeError as exc:
        logging.error("%s", exc)
        return 1

    cache: Dict[str, str] = {}
    total_files = 0
    modified_files = 0

    for file_path in iter_source_files(root, {ext.lower() for ext in args.extensions}):
        total_files += 1
        if process_file(file_path, translator, cache, dry_run=args.dry_run):
            modified_files += 1

    logging.info(
        "Processed %d files. %s %d file(s).",
        total_files,
        "Would modify" if args.dry_run else "Modified",
        modified_files,
    )
    return 0


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    sys.exit(main(sys.argv[1:]))


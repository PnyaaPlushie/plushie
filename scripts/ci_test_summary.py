#!/usr/bin/env python3
"""
Write a markdown summary of CTest JUnit results for a GitHub Actions job.

Usage: ci_test_summary.py <title> [junit.xml ...]

Each JUnit file is reported as one configuration (named after the file, e.g.
ctest-junit-Debug.xml -> Debug). Missing files are ignored, so a matrix entry
which never ran tests (or failed before testing) reports "No tests were run".

The markdown is appended to $GITHUB_STEP_SUMMARY when set, otherwise printed.
"""

import os
import sys
import xml.etree.ElementTree as ET

MAX_OUTPUT_CHARS = 4000


def config_name(path):
    name = os.path.splitext(os.path.basename(path))[0]
    prefix = "ctest-junit-"
    return name[len(prefix):] if name.startswith(prefix) else name


def is_intentional_skip(case):
    # CTest writes "notrun" tests as <skipped message="...">. Only skip return
    # codes / skip regexes / disabled tests are real skips; anything else
    # ("Unable to find executable", "Fixture dependency failed", ...) is
    # counted as a failure by CTest, so report it as one here too.
    if (case.get("status") or "").lower() == "disabled":
        return True
    skipped = case.find("skipped")
    message = (skipped.get("message") or "") if skipped is not None else ""
    return message.startswith("SKIP_") or message == "Disabled"


def parse(path):
    root = ET.parse(path).getroot()
    result = {"passed": [], "failed": [], "skipped": []}
    for case in root.iter("testcase"):
        name = case.get("name", "<unnamed>")
        status = (case.get("status") or "").lower()
        if status == "run" and case.find("failure") is None:
            result["passed"].append(name)
        elif case.find("skipped") is not None and is_intentional_skip(case) or status == "disabled":
            result["skipped"].append(name)
        else:
            out = case.findtext("system-out") or ""
            result["failed"].append((name, out))
    return result


def code_block(text):
    text = text.strip()
    if len(text) > MAX_OUTPUT_CHARS:
        text = "...(truncated)...\n" + text[-MAX_OUTPUT_CHARS:]
    return "```\n" + text.replace("```", "'''") + "\n```"


def main(argv):
    if len(argv) < 2:
        print(__doc__, file=sys.stderr)
        return 2

    title, paths = argv[1], [p for p in argv[2:] if os.path.isfile(p)]
    lines = [f"## Tests: {title}", ""]

    if not paths:
        lines += ["No tests were run for this configuration.", ""]
    else:
        lines += [
            "| Configuration | Total | Passed | Failed | Skipped |",
            "|---|---:|---:|---:|---:|",
        ]
        results = []
        for path in sorted(paths):
            try:
                r = parse(path)
            except ET.ParseError as e:
                lines.append(f"| {config_name(path)} | - | - | - | - |")
                print(f"warning: could not parse {path}: {e}", file=sys.stderr)
                continue
            results.append((config_name(path), r))
            total = sum(len(v) for v in r.values())
            icon = ":x:" if r["failed"] else ":white_check_mark:"
            lines.append(
                f"| {icon} {config_name(path)} | {total} | {len(r['passed'])} "
                f"| {len(r['failed'])} | {len(r['skipped'])} |"
            )
        lines.append("")

        failures = [(cfg, name, out) for cfg, r in results for name, out in r["failed"]]
        if failures:
            lines += ["### Failed tests", ""]
            for cfg, name, out in failures:
                if out.strip():
                    lines += [
                        f"<details><summary>:x: <code>{name}</code> ({cfg})</summary>",
                        "",
                        code_block(out),
                        "",
                        "</details>",
                    ]
                else:
                    lines.append(f"- :x: `{name}` ({cfg})")
            lines.append("")

    text = "\n".join(lines) + "\n"
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a", encoding="utf-8") as f:
            f.write(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

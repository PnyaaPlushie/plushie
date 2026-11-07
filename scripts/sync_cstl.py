#!/usr/bin/env python3
"""
Mirror a subfolder of the monorepo to its own repository.

Default: modules/plushie_cstl -> remote "cstl", branch "main".

    python sync_cstl.py
    python sync_cstl.py --url git@github.com:you/plushie_cstl.git
    python sync_cstl.py --prefix modules/plushie_cstl --remote cstl --branch main
    python sync_cstl.py --dry-run

The split is deterministic, so repeated syncs are plain fast-forward pushes.
Uses splitsh-lite if it's on PATH (much faster on long histories), otherwise
falls back to `git subtree split`.
"""

import argparse
import re
import shutil
import subprocess
import sys

DEFAULT_PREFIX = "modules/plushie_cstl"
DEFAULT_REMOTE = "cstl"
DEFAULT_URL = "https://github.com/PnyaaPlushie/plushie_cstl" 
DEFAULT_BRANCH = "main"
EXPORT_BRANCH = "cstl-export"
SYNC_BRANCH = "monorepo-sync"  # branch on the mirror that the PR comes from


def git(*args, check=True, capture=True, cwd=None):
    result = subprocess.run(
        ["git", *args],
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE if capture else None,
        stderr=subprocess.PIPE if capture else None,
    )
    if check and result.returncode != 0:
        err = (result.stderr or "").strip()
        sys.exit(f"error: git {' '.join(args)} failed\n{err}")
    return result


def repo_root():
    r = git("rev-parse", "--show-toplevel", check=False)
    if r.returncode != 0:
        sys.exit("error: not inside a git repository")
    return r.stdout.strip()


def ensure_remote(root, name, url):
    """Add the remote, or fix its URL if it exists but points elsewhere."""
    r = git("remote", "get-url", name, check=False, cwd=root)
    if r.returncode != 0:
        print(f"adding remote '{name}' -> {url}")
        git("remote", "add", name, url, cwd=root)
        return

    current = r.stdout.strip()
    if current == url:
        print(f"remote '{name}' already set -> {url}")
    else:
        print(f"remote '{name}' points to {current}, updating -> {url}")
        git("remote", "set-url", name, url, cwd=root)


def warn_uncommitted(root, prefix):
    r = git("status", "--porcelain", "--", prefix, cwd=root)
    if r.stdout.strip():
        print(f"warning: uncommitted changes under {prefix} will NOT be synced:")
        print(r.stdout.rstrip())


def split(root, prefix):
    if shutil.which("splitsh-lite"):
        print("splitting with splitsh-lite...")
        r = subprocess.run(
            ["splitsh-lite", f"--prefix={prefix}", f"--target=refs/heads/{EXPORT_BRANCH}"],
            cwd=root, text=True, capture_output=True,
        )
        if r.returncode != 0:
            sys.exit(f"error: splitsh-lite failed\n{r.stderr.strip()}")
    else:
        print("splitting with git subtree (install splitsh-lite for speed)...")
        # Delete the old export branch so subtree doesn't complain about it.
        git("branch", "-D", EXPORT_BRANCH, check=False, cwd=root)
        git("subtree", "split", f"--prefix={prefix}", "-b", EXPORT_BRANCH, cwd=root)

    return git("rev-parse", EXPORT_BRANCH, cwd=root).stdout.strip()


def gh_repo_from_url(url):
    """git@github.com:you/repo.git / https://github.com/you/repo -> you/repo"""
    m = re.search(r"github\.com[:/]+([^/]+/[^/]+?)(?:\.git)?/?$", url)
    if not m:
        sys.exit(f"error: can't work out a GitHub owner/repo from '{url}'")
    return m.group(1)


def already_merged(root, remote, base, sha):
    """True if the split head is already in the mirror's base branch."""
    if git("fetch", remote, base, check=False, cwd=root).returncode != 0:
        return False  # empty mirror or no base branch yet
    return git("merge-base", "--is-ancestor", sha, "FETCH_HEAD", check=False, cwd=root).returncode == 0


def open_or_update_pr(repo, base, head):
    if not shutil.which("gh"):
        print(f"gh not installed. Branch '{head}' was pushed; open the PR manually:")
        print(f"  https://github.com/{repo}/compare/{base}...{head}")
        return

    r = subprocess.run(
        ["gh", "pr", "list", "--repo", repo, "--head", head, "--base", base,
         "--state", "open", "--json", "url", "--jq", ".[0].url"],
        text=True, capture_output=True,
    )
    if r.returncode != 0:
        sys.exit(f"error: gh pr list failed (run `gh auth login`?)\n{r.stderr.strip()}")
    if r.stdout.strip():
        print(f"updated existing PR: {r.stdout.strip()}")
        return

    r = subprocess.run(
        ["gh", "pr", "create", "--repo", repo, "--base", base, "--head", head,
         "--title", "Sync from monorepo",
         "--body", "Automated sync of `plushie_cstl` from the Plushie monorepo.\n\n"
                   "Merge with **Create a merge commit** (not squash/rebase) so the next "
                   "sync stays a clean continuation of this history."],
        text=True, capture_output=True,
    )
    if r.returncode != 0:
        sys.exit(f"error: gh pr create failed\n{r.stderr.strip()}")
    print(f"opened PR: {r.stdout.strip()}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--prefix", default=DEFAULT_PREFIX)
    ap.add_argument("--remote", default=DEFAULT_REMOTE)
    ap.add_argument("--url", default=DEFAULT_URL)
    ap.add_argument("--branch", default=DEFAULT_BRANCH, help="branch on the mirror repo")
    ap.add_argument("--dry-run", action="store_true", help="split but don't push")
    ap.add_argument("--direct", action="store_true", help="push straight to --branch instead of opening a PR")
    args = ap.parse_args()

    root = repo_root()
    prefix = args.prefix.replace("\\", "/").strip("/")

    if git("ls-tree", "-d", "HEAD", prefix, cwd=root).stdout.strip() == "":
        sys.exit(f"error: '{prefix}' doesn't exist in HEAD")

    ensure_remote(root, args.remote, args.url)
    warn_uncommitted(root, prefix)

    sha = split(root, prefix)
    print(f"split head: {sha}")

    if already_merged(root, args.remote, args.branch, sha):
        print(f"{args.remote}/{args.branch} already contains this split, nothing to sync.")
        return

    if not args.direct:
        repo = gh_repo_from_url(args.url)
        if args.dry_run:
            print(f"dry run: would push {EXPORT_BRANCH} -> {args.remote}/{SYNC_BRANCH} and open a PR into {args.branch}")
            return
        print(f"pushing to {args.remote}/{SYNC_BRANCH}...")
        # Force is fine here: the sync branch is owned by this script.
        git("push", "--force", args.remote, f"{EXPORT_BRANCH}:refs/heads/{SYNC_BRANCH}", capture=False, cwd=root)
        open_or_update_pr(repo, args.branch, SYNC_BRANCH)
        return

    if args.dry_run:
        print(f"dry run: would push {EXPORT_BRANCH} -> {args.remote}/{args.branch}")
        return

    print(f"pushing to {args.remote}/{args.branch}...")
    r = git("push", args.remote, f"{EXPORT_BRANCH}:{args.branch}", check=False, capture=False, cwd=root)
    if r.returncode != 0:
        sys.exit(
            "error: push rejected. The mirror probably has commits that aren't in the\n"
            "monorepo (someone committed there directly). Pull those changes into the\n"
            "monorepo, or force-push if the mirror is meant to be strictly read-only:\n"
            f"  git push --force {args.remote} {EXPORT_BRANCH}:{args.branch}"
        )
    print("done.")


if __name__ == "__main__":
    main()
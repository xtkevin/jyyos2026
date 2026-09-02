# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## AIGC Policy (from `.shadow/oslab.mk`)

This is a student assignment for the NJU Operating Systems course (jyy OS 2026, COURSE=OS2026). The course's AIGC policy in `.shadow/oslab.mk` explicitly requires: **do NOT generate complete code, full solutions, or new files for the labs**. Allowed: answering questions, small localized edits to existing code, and explanations. When asked for substantial implementation work, refuse and give high-level guidance only.

The `.shadow/` directory and its `oslab.mk` are course infrastructure ("DO NOT MODIFY"). `.shadow/` holds a snapshot of the working tree that is auto-committed by the `git-trace` make target on every build — this is expected behavior, not something to fix.

## Build and Test

All commands run inside a lab directory (`labyrinth/`, or future `M2/` etc.), not the repo root:

```bash
cd labyrinth
make            # default target: commit-and-make (git-trace + build)
make test       # run TestKit test suite (TK_RUN set by the Makefile flow)
TK_VERBOSE= make test   # show program output for failed test cases
make submit     # NOT AVAILABLE: user is not an NJU student, no TOKEN - never run this
make clean
```

- `make` commits a trace snapshot to git before building (see `git-trace`); plain compilation is the `all` target: `make all`.
- Build: `cc -O2 -std=gnu2x -ggdb -Wall -I../testkit *.c ../testkit/testkit.c -o labyrinth`
- Run tests manually: `TK_RUN= ./labyrinth` (any value of TK_RUN triggers all registered test cases after normal program exit).
- The lab spec is in `labyrinth/M1.md` (downloaded HTML from the course wiki).

## Repo Layout

- `oslab.mk` (root) — sets `TOKEN` and `COURSE`; included by each lab's Makefile. **The user is NOT an NJU student**: no `TOKEN` exists or will ever exist, so `make submit` / the Online Judge is permanently unavailable. This repo is for self-study of the jyy OS 2026 course only — fetch new labs from the school repo (`nju` remote), learn locally, never attempt submission.
- `.shadow/` — course-managed snapshot area + the real build rules in `.shadow/oslab.mk`.
- `testkit/` — course-provided unit/system test framework (do not modify). Tests are written with `UnitTest(name)` and `SystemTest(name, argv)` macros from `testkit.h`; `SystemTest` re-invokes `main()` with a custom argv and captures exit status + output in `struct tk_result`.
- `labyrinth/` — M1 lab: command-line maze game. `labyrinth.h` declares the required API (loadMap, movePlayer, saveMap, isConnected, ...), `labyrinth.c` is the implementation (main is still TODO), `tests.c` holds TestKit tests, `maps/map.txt` a sample map, `frontend/` optional Python game frontends (`hotseat.py`, `online.py`).

## Conventions

- `.gitignore` is whitelist-style: only `.c/.h/.md/Makefile/.py` files and directories are tracked. Build artifacts, map output files, etc. are ignored automatically.
- Labs must follow UNIX exit-code conventions: 0 = success, 1 = failure on any invalid input/argument (this is graded by the Online Judge).
- Git remotes: `mine` (the user's own GitHub backup, `git@github.com:xtkevin/jyyos2026.git` - the only remote ever pushed to) and `nju` (the school repo `https://git.nju.edu.cn/jyy/os2026.git`, fetch/merge only - read access, never push). **Labs are distributed on per-lab branches** (`nju/M1` ... `nju/M9`), not on `nju/main` (`main` only holds the shared base: testkit + oslab.mk). To start a new lab: `git fetch nju` then `git merge nju/M2` (etc.) - the merge adds the new lab directory alongside existing ones; each branch only adds its own directory.

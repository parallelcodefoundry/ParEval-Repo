# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

ParEval-Repo is an LLM benchmarking suite for repository-scale translation of HPC (parallel) codes. It translates codebases between parallel programming models (e.g., CUDA → Kokkos, OpenMP-offload → CUDA) using LLMs, then builds and validates the translated code on HPC systems.

## Setup

```bash
# Preferred (uses exact pinned versions)
uv sync && . .venv/bin/activate

# Alternative
pip install -r requirements.txt
```

Python 3.11.13+ required.

## Key Commands

### Translation (LLM inference)
```bash
python src/translate/translate.py --help

# Example: naive translation
python src/translate/translate.py \
  --input targets/XSBench/openmp-offload \
  --output /path/to/output \
  --src-model openmp-offload \
  --dst-model cuda \
  --method naive \
  --config config/perlmutter-config.json
```

### Running drivers (build and test translated repos)
```bash
python src/drivers/run-all.py --help

# Example
python src/drivers/run-all.py \
  --translations-root /path/to/translations \
  --output results.json \
  --config config/perlmutter-config.json
```

## Architecture

### Two-Phase Pipeline

1. **Translation phase** (`src/translate/`): Takes a source repo + `target.json` for both input and output, calls an LLM to produce translated code files.
2. **Driver phase** (`src/drivers/`): Builds and runs the translated repos, comparing outputs against expected values.

### Translation Methods (`src/translate/`)

Three strategies, selectable via `--method`:

- **naive** (`naive/`): Translates file-by-file with full repo context in a single LLM prompt. Uses `ChunkFileAgent` for large files.
- **top-down-agentic** (`top_down_agentic/`): Multi-agent pipeline: `DependencyAgent` builds a file dependency tree → `ChunkAgent` splits large files → `ContextAgent` gathers relevant context → translates each file.
- **swe-agent** (`swe_agent/`): Wraps the external SWE-agent tool for autonomous translation.

All methods inherit from `Translator` ABC (`translator.py`) and use `GeneratorMixin` (`generator_mixin.py`) for unified LLM access across backends (OpenAI, Gemini, HuggingFace, vLLM, local).

### Target Configuration (`target.json`)

Each `targets/<app>/<model>/` directory requires a `target.json` with:
- Build/run commands and timeouts
- Expected output strings for validation (`debug_outputs`, `debug_type`)
- File classifications (build files, main entry points)
- Dependency module names (resolved via system config)

The driver reads `target.json` to know how to build, run, and validate each translated repo.

### System Configuration (`config/`)

JSON files per HPC system (e.g., `perlmutter-config.json`) that map dependency names to module load commands and set GPU architecture (`sm`). Passed to both translation and driver scripts via `--config`.

### Driver Utilities (`src/drivers/util.py`)

Core utility classes used throughout drivers:
- `CommandExecutor` — runs shell commands with timeout and dry-run support
- `ConfigManager` — loads and resolves system config
- `DataManager` — persists results to JSON
- `ResultBuilder` — constructs structured build/run result objects

## Adding a New Target

1. Create `targets/<app>/<model>/repo/` with source code
2. Create `targets/<app>/<model>/target.json` following the schema in existing targets
3. Provide a `target.json` for both source and destination models when translating

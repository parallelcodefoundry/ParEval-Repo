# Code-Translation

This repository contains files related to the LLMs for code translation project.

The layout of the repository is as follows:

- `src/`: Implementations of each translation method including RAG and LLM
  inference.
- `targets/`: Repositories that will be translated from one programming model
  to another. Further organized as `targets/<repo>/<programming model>/`.
- `eval/`: Scripts and test to automatically evaluate each of the targets.
  Further organized as `eval/<repo>/test.sh`.
- `data/`: Data from translation experiments.
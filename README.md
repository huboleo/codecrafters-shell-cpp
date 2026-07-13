# POSIX-Compliant Shell

A POSIX compliant shell written from scratch in C++23.

Project was built following CodeCrafters Build your own Shell challenge available [here](https://app.codecrafters.io/courses/shell/overview).

If you are interested in building low-level projects I highly recommend to check them out, they cool.

## Features

- Runs external programs discovered through `PATH`.
- Provides built-in commands: `cd`, `complete`, `declare`, `echo`, `exit`, `history`, `jobs`, `pwd`, and `type`.
- Parses single and double quotes, escaped characters, and shell variables (`$NAME` and `${NAME}`).
- Supports multi-command pipelines with `|`.
- Redirects standard output and standard error with `>`, `>>`, `1>`, `1>>`, `2>`, and `2>>`.
- Runs commands and pipelines in the background with `&`, including job tracking and completion notices.
- Maintains interactive and file-backed command history through `HISTFILE`.
- Offers tab completion for built-ins, executables, files, directories, and command-specific completion scripts.

## Testing
Extensive test suite was provided by codecrafters for all 76 stages, all tests passed.

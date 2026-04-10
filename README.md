\# Minishell



\## Overview

A custom Linux shell in C that accepts user commands, identifies 

them as builtin or external and executes them.



\## Features

\- Supports builtin commands (cd, pwd, echo, exit, jobs, fg, bg)

\- Executes external commands using fork/execvp

\- Supports piped commands (cmd1 | cmd2)

\- Signal handling (SIGINT, SIGTSTP, SIGCHLD)

\- Background and foreground job control

\- Customizable PS1 prompt



\## Files

| File | Description |

|------|-------------|

| `header.h` | Structs, macros and function declarations |

| `commands.c` | Builtin and external command detection and execution |

| `scan\_input.c` | Input scanning, signal handling and job control |

| `main.c` | Entry point with prompt initialization |

| `external.txt` | List of supported external commands |



\## Compilation

```bash

gcc main.c commands.c scan\_input.c -o minishell

```



\## Usage

```bash

./minishell

```



\## Tech Stack

\- Language: C

\- Concepts: fork, execvp, signals, pipes, job control




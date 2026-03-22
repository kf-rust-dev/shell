# shell

Simple shell written in C

## Features

- Execute system commands
- Built-in commands: `cd`, `pwd`, `echo`, `clear`, `history`, `help`, `exit`, 
- Persistent command history using GNU Readline
- Input parser that supports:
  - Single quotes `'...'` and double quotes `"..."` to include spaces in tokens
  - Escaped characters `\` within tokens
  - Detection of unmatched quotes to prevent shell crashes
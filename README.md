# gccsh
use C in the shell (WIP)
### features
- execute C code in a shell-like environment
- familiar builtins like `chdir` and `exit`
- `cmd` builtin allows you to run a command in the default shell and returns the output.
- `version` variable is provided as a c string
### known issues
- can only use `chdir` and `exit`  properly once and it will always run before the rest of the code


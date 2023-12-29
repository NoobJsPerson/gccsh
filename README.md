# gccsh
use C in the shell (WIP)
### features
- execute C code in a shell-like environment
- familiar builtins like `chdir` and `exit`
- `gccsh_profile` file gets ran whenever you run a command and it gets placed in the main function. you can add functions in it to be always defined when you run any command.
- `cmd` builtin allows you to run a command in the default shell and returns the output.
- the `GCCSH_VERSION` environment variable while the program is running
### known issues
- can only use `chdir` and `exit`  properly once and it will always run before the rest of the code
### credits
- This program is inspired and based off [Stephen Brennan's LSH](https://github.com/brenns10/lsh). You can checkout his tutorial [here](https://brennan.io/2015/01/16/write-a-shell-in-c/)

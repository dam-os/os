- Involves switching context between 2 functions (processes)
- Each function has its own set of values in their registries
- Need to save callee-saved registers `ra` and `s0-s11` (#todo VERIFY that it's
  not caller-saved)
- Decide to do this by saving processes as structs, with register data saved on
  the struct.
- Need to save callee-saved registers `ra` and `s0-s11` (#todo VERIFY that it's
  not caller-saved)
- Decide to do this by saving processes as structs, with register data saved on
  the struct.
- Attempts at saving this:
  1. Use inline assembly and variable substitution to save/load registers.
     - Saving works, but loading does not. Issues arise as compiler tries to
       use\
       `s0` register for memory lookups after it has been loaded to a 0.
  2. Use naked assembly to save/load directly onto the structs by looking at\
     `a0` and `a1` registers, which are function arguments.
     - Debugging reveals that none of the function argument registers contain\
       the function arguments, so need to investigate why it doesn't align with\
       the RISC-V calling conventions.
       - All `a*` registers contain the address `0x800200` (#todo VERIFY),\
         except one, which contains a seemingly random memory address
       - Perhaps the debugger is wrong? Maybe GDB sees incorrect registry\
         values?
- Turns out the `create_process` assigns all functions to the same address,
  because I forgot to set the process state to something other than
  `PROCESS_EMPTY`.
- Processes seem to return to the wrong process after yield
  - This is because all processes share the same stack
  - Implement a stack for each process
- Stack switching does not seem to work
  - I had initialised the stack pointer incorrectly
- For some reason, a process repeats if its function returns
  - When the function call is started, the function itself is sat to `ra`.
  - This is because `ret` was never meant to return to the start of a function,
    it was meant to return to a function call!
- Fix:
  - Only use `ret` when processes return from calling `yield`.
  - When a process is started
    - Set `ra` to custom `exit_proc` function that handles exits
      - This causes top-level function to return to `exit_proc`
    - Manually jump to process entrypoint.
  - We can tell the difference b; adding a new state, `PROCESS_READY`,
    indicating a process not yet running, but ready to start.

# pageboy

`pageboy` is a memory-mapped data store that uses a B-Tree to organize records in pages on-disk. The frontend supports a query language and REPL interface, converting instructions into byte code sequences that are then executed by a core virtual machine. This virtual machine is a finite state machine (FSM) interlaced with a statement preparer.

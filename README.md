# pageboy

`pageboy` is a memory-mapped data store that utilizes a B-Tree to organize records in pages on-disk.

The frontend supports a query language and REPL interface, converting instructions into byte code sequences that are then executed by a core virtual machine.

This virtual machine is a finite state machine interlaced with a statement preparer.

## Components

- REPL: frontend interface for query language execution
- Virtual Machine: state machine for reducing prepared statements into scalar primitives
- Preparer: state machine for creating prepared statements that are then sent to the VM to be executed
- Pager: responsible for memory mapping and process management
- Cursor: tbd

## Test Coverage

`pageboy` is tested with `shpec` and a custom `TAP` harness

Invoke test suite(s):

```shell
shpec __tests__/main.shpec.bash
```

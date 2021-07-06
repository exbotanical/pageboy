#include "repl.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	InputBuffer* ib = ib_init();

	while (1) {
		rprompt();
		ib_read(ib);

		if (ib->buffer[0] == '.') {
			switch(proc_meta_cmd(ib)) {
				case META_CMD_SUCCESS:
					continue;
				case META_CMD_UNRECOGNIZED_CMD:
					fprintf(stderr, "Unrecognized command '%s'\n", ib->buffer);
					continue;
			}
		}

		Statement statement;

		switch (prepare_statement(ib, &statement)) {
			case PREPARE_SUCCESS:
				break;
			case PREPARE_E_UNRECOGNIZED_STMT:
				fprintf(
					stderr,
					"Unrecognized keyword at start of '%s'\n",
					ib->buffer
				);
				continue;
		}

		exec_statement(&statement);
		fprintf(stdout, "%s\n", "Executed statement");
	}

	return EXIT_SUCCESS;
}

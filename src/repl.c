#include "repl.h"

#include "common.h"

#include <stdlib.h>
#include <stdio.h>

/* Input Buffer Ctrl */

/**
 * @brief Initialize a new input buffer
 *
 * @return InputBuffer*
 */
InputBuffer* ib_alloc(void) {
	InputBuffer* ib = malloc(sizeof(InputBuffer));
	ib->buffer = NULL;
	ib->buffer_l = 0;
	ib->input_l = 0;

	return ib;
}

/**
 * @brief Deallocate input buffer
 *
 * @param ib
 */
void ib_free(InputBuffer* ib) {
	free(ib->buffer);
	free(ib);
}

/**
 * @brief Read stdin into an input buffer
 *
 * @param ib
 */
void ib_read(InputBuffer* ib) {
	ssize_t bytes;

	if ((bytes = getline(
		&(ib->buffer),
		&(ib->buffer_l),
		stdin
	)) <= 0) DIE("%s\n", "Error reading input");

	ib->input_l = bytes - 1;
	ib->buffer[bytes - 1] = '\0';
}

/* Stdout + Prompts */

/**
 * @brief REPL prompt prefix
 */
void rprompt(void) {
	printf("%s > ", APP_NAME);
}

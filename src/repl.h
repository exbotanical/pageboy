#ifndef REPL_H
#define REPL_H

#include <unistd.h>

/* Structures */

typedef struct {
	char* buffer; /**< store actual user input */
	size_t buffer_l; /**< store size of user input */
	ssize_t input_l; /**< ?*/
} InputBuffer;

/* Functions */

InputBuffer *ib_alloc(void);

void ib_free(InputBuffer *ib);

void ib_read(InputBuffer *ib);

void rprompt(void);

#endif

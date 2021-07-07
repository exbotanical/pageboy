#include "serialize.h"

#include <stdlib.h>
#include <string.h>

void serialize_row(Row* src, void* dest) {
	memcpy(
		dest + ID_OFFSET,
		&(src->id),
		ID_SIZE
	);

	memcpy(
		dest + UNAME_OFFSET,
		&(src->uname),
		UNAME_SIZE
	);

	memcpy(
		dest + EMAIL_OFFSET,
		&(src->email),
		EMAIL_SIZE
	);
}

void deserialize_row(void* src, Row* dest) {
	memcpy(
		&(dest->id),
		src + ID_OFFSET,
		ID_SIZE
	);

	memcpy(
		&(dest->uname),
		src + UNAME_OFFSET,
		UNAME_SIZE
	);

	memcpy(
		&(dest->email),
		src + EMAIL_OFFSET,
		EMAIL_SIZE
	);
}

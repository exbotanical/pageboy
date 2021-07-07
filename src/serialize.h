#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "table.h"

void serialize_row(Row *src, void *dest);

void deserialize_row(void* src, Row* dest);

#endif

#ifndef UTILS_H
#define UTILS_H

#include "../modules/rfid.h"
#include "../modules/rf.h"

void dump_byte_array1(byte *buffer, byte bufferSize);
void dump_byte_array(byte *buffer, byte bufferSize);
void hexdump(const void *mem, uint32_t len, uint8_t cols);
void initializeModule(int a);

#endif
#pragma once
#include "stdint.h"

#define __cdecl __attribute__((cdecl))

void __cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);

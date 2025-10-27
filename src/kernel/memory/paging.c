#include "paging.h"

typedef struct {
    uint32_t entry[1024];
} __attribute__((aligned(4096))) PageTable;

PageTable **allocated_tables;
int table_count;

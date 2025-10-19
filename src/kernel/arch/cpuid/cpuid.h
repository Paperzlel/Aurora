#pragma once

#include <stdint.h>
#include <stdbool.h>

// 64-bit enums are a C11 thing, sadly

typedef enum {
    // EDX values

    CPU_FEATURE_X87 =                   1 << 0,
    CPU_FEATURE_V86_EXT =               1 << 1,
    CPU_FEATURE_DEBUG_EXT =             1 << 2,
    CPU_FEATURE_PAGE_SIZE_EXT =         1 << 3,
    CPU_FEATURE_TIMESTAMP_COUNTER =     1 << 4,
    CPU_FEATURE_MSRS =                  1 << 5,
    CPU_FEATURE_PHYS_ADDR_EXT =         1 << 6,
    CPU_FEATURE_MACHINE_CHECK_ERR =     1 << 7,
    CPU_FEATURE_CMP_SWAP =              1 << 8,
    CPU_FEATURE_APIC =                  1 << 9,
    CPU_FEATURE_RESERVED =              1 << 10,
    CPU_FEATURE_FAST_SYSCALLS =         1 << 11,
    CPU_FEATURE_MTRR =                  1 << 12,
    CPU_FEATURE_PAGE_GLOB_ENABLE =      1 << 13,
    CPU_FEATURE_MACHINE_CHECK_ARCH =    1 << 14,
    CPU_FEATURE_CONDITIONAL_MOVE =      1 << 15,
    CPU_FEATURE_PAGE_ATTRIBUTE_TABLE =  1 << 16,
    CPU_FEATURE_PSE_36 =                1 << 17,
    CPU_FEATURE_PROCESSOR_SERIAL_NO =   1 << 18,
    CPU_FEATURE_CLFLUSH =               1 << 19,
    CPU_FEATURE_NO_EXECUTE =            1 << 20,
    CPU_FEATURE_DEBUG_STORE =           1 << 21,
    CPU_FEATURE_ACPI =                  1 << 22,
    CPU_FEATURE_MMX =                   1 << 23,
    CPU_FEATURE_FXSR =                  1 << 24,
    CPU_FEATURE_SSE =                   1 << 25,
    CPU_FEATURE_SSE2 =                  1 << 26,
    CPU_FEATURE_SELF_SNOOP =            1 << 27,
    CPU_FEATURE_HTT =                   1 << 28,
    CPU_FEATURE_THERMAL_MONTIOR =       1 << 29,
    CPU_FEATURE_IA64 =                  1 << 30,
    CPU_FEATURE_PENDING_BREAK_ENABLE =  1 << 31,

    // ECX values

    CPU_FEATURE_SSE3 =                  1 << 0,
    CPU_FEATURE_PCLMULQDQ =             1 << 1,
    CPU_FEATURE_DEBUG_STORE_64 =        1 << 2,
    CPU_FEATURE_MONITOR =               1 << 3,
    CPU_FEATURE_DEBUG_STORE_CPL =       1 << 4,
    CPU_FEATURE_VMX =                   1 << 5,
    CPU_FEATURE_SMX =                   1 << 6,
    CPU_FEATURE_SPEED_STEP =            1 << 7,
    CPU_FEATURE_THERMAL_MONITOR_2 =     1 << 8,
    CPU_FEATURE_SSSE3 =                 1 << 9,
    CPU_FEATURE_L1_CONTEXT_ID =         1 << 10,
    CPU_FEATURE_SILICON_DEBUG_INTF =    1 << 11,
    CPU_FEATURE_FMA3 =                  1 << 12,
    CPU_FEATURE_CX16 =                  1 << 13,
    CPU_FEATURE_XTPR =                  1 << 14,
    CPU_FEATURE_PDCM =                  1 << 15,
    CPU_FEATURE_RESERVED2 =             1 << 16,
    CPU_FEATURE_PROC_CTX_IDS =          1 << 17,
    CPU_FEATURE_DIRECT_CACHE_ACCESS =   1 << 18,
    CPU_FEATURE_SSE4_1 =                1 << 19,
    CPU_FEATURE_SSE4_2 =                1 << 20,
    CPU_FEATURE_X2APIC =                1 << 21,
    CPU_FEATURE_MOVBE =                 1 << 22,
    CPU_FEATURE_POPCNT =                1 << 23,
    CPU_FEATURE_TSC_DEADLINE =          1 << 24,
    CPU_FEATURE_AES_NI =                1 << 25,
    CPU_FEATURE_XSAVE =                 1 << 26,
    CPU_FEATURE_OSXSAVE =               1 << 27,
    CPU_FEATURE_AVX =                   1 << 28,
    CPU_FEATURE_F16C =                  1 << 29,
    CPU_FEATURE_RDRAND =                1 << 30,
    CPU_FEATURE_HYPERVISOR =            1 << 31
} CPU_Features;


typedef struct {
    char vendor_name[13];
    char model_name[49];
    uint32_t features_edx;
    uint32_t features_ecx;
    uint32_t max_function_param;
    uint8_t sse_version;
    uint8_t family_id;
    uint8_t model_id;
    
    uint8_t local_apic_id;

    bool can_use_msrs;
} CPU_Config;

bool cpuid_supports_feature(CPU_Features p_feature, int reg);

bool cpuid_initialize(CPU_Config *out_config);

char *cpuid_get_features();
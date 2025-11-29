#include "cpuid.h"

#include <cpuid.h>
#include <string.h>

const char *feature_list[] = {
    "fpu",
    "vme",
    "de",
    "pse",
    "tsc",
    "msr",
    "pae",
    "mce",
    "cx8",
    "apic",
    "",
    "sep",
    "mtrr",
    "pge",
    "mca",
    "cmov",
    "pat",
    "pse-36",
    "psn",
    "clfsh",
    "(nx)",
    "ds",
    "acpi",
    "mmx",
    "fxsr",
    "sse",
    "sse2",
    "ss",
    "htt",
    "tm",
    "ia64",
    "pbe",
    "sse3",
    "pclmulqdq",
    "dtes64",
    "monitor",
    "ds-cpl",
    "vmx",
    "smx",
    "est",
    "tm2",
    "ssse3",
    "cnxt-id",
    "sdbg",
    "fma",
    "cx16",
    "xtpr",
    "pdcm",
    "",
    "pcid",
    "dca",
    "sse4.1",
    "sse4.2",
    "x2apic",
    "movbe",
    "popcnt",
    "tsc-deadline",
    "aes-ni",
    "xsave",
    "osxsave",
    "avx",
    "f16c",
    "rdrnd",
    "hypervisor",
    0
};

static char list[1024];

static uint32_t edx_features;
static uint32_t ecx_features;

/**
 * @brief Checks if the given CPU feature is supported. Useful for checking if we can use features such as SSE, x87 and so on.
 * @param p_feature The feature to check for. Defined in the header.
 * @param reg The register to check from. If the value is in ECX, this is 0. If the value is in EDX, this is 1.
 */
bool cpuid_supports_feature(CPU_Features p_feature, int reg) {
    if (reg > 0 && edx_features & p_feature) {
        return true;
    }

    if (reg <= 0 && ecx_features & p_feature) {
        return true;
    }

    return false;
}

bool cpuid_initialize(CPU_Config *out_config) {
    if (!out_config) {
        return false;
    }
    
    unsigned int regs[4];
    char vendor_id[13] = {0};
    __get_cpuid(0, regs, &regs[1], &regs[3], &regs[2]);

    for (int i = 0; i < 3; i++) {
        memcpy(vendor_id + (i * 4), &regs[i + 1], 4);
    }
    // Copy vendor name into CPUID
    memcpy(out_config->vendor_name, vendor_id, 13);
    out_config->max_function_param = regs[0];

    // Check features and processor info
    __get_cpuid(1, regs, &regs[1], &regs[2], &regs[3]);
    ecx_features = regs[2];
    edx_features = regs[3];
    out_config->features_ecx = ecx_features;
    out_config->features_edx = edx_features;

    uint32_t eax = regs[0];
    out_config->family_id = eax >> 8;
    out_config->model_id = eax >> 4;
    if ((eax >> 8) == 15 || (eax >> 8) == 6) {
        uint32_t tmp = eax >> 16;
        tmp <<= 4;
        out_config->model_id = tmp + (eax >> 4);

        if ((eax >> 8) == 15) {
            out_config->family_id = (eax >> 20) + (eax >> 8);
        }
    }

    // Set APIC location
    if (out_config->family_id >= 5 && edx_features & CPU_FEATURE_APIC) {
        out_config->local_apic_id = regs[1] >> 24;
    }

    int version = 0;
#define HAS_SSE_VERSION(x, y) if (cpuid_supports_feature(CPU_FEATURE_SSE##x, y)) { version++; }

    HAS_SSE_VERSION(, 1);
    HAS_SSE_VERSION(2, 1);
    HAS_SSE_VERSION(3, 0);
    HAS_SSE_VERSION(4_1, 0);
    HAS_SSE_VERSION(4_2, 0);

#undef HAS_SSE_VERSION

    out_config->sse_version = version;
    out_config->can_use_msrs = cpuid_supports_feature(CPU_FEATURE_MSRS, 1);

    unsigned int string[12];
    __get_cpuid(0x80000000, &string[0], &string[1], &string[2], &string[3]);

    if (regs[0] >= 0x8000004) {
        __get_cpuid(0x80000002, &string[0], &string[1], &string[2], &string[3]);
        __get_cpuid(0x80000003, &string[4], &string[5], &string[6], &string[7]);
        __get_cpuid(0x80000004, &string[8], &string[9], &string[10], &string[11]);

        memcpy(out_config->model_name, string, 48);
    } else {
        // Make empty string
        memset(out_config->model_name, 0, 48);
    }
    out_config->model_name[48] = 0;

    return true;
}

char *cpuid_get_features() {
    if (list[0] != 0) {
        return list;
    }

    int idx = 0;

    for (int i = 0; i < 32; i++) {
        if (edx_features & (1 << i)) {
            strcpy(list + idx, feature_list[i]);
            idx += strlen(feature_list[i]);
            list[idx] = ' ';
            idx++;
        }
    }

    for (int i = 0; i < 32; i++) {
        if (ecx_features & 1 << i) {
            strcpy(list + idx, feature_list[i + 32]);
            idx += strlen(feature_list[i + 32]);
            list[idx] = ' ';
            idx++;
        }
    }

    return list;
}
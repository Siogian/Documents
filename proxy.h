#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
    E_OK,
    E_NOT_OK,
    DCM_PENDING,
    E_INITIAL
} Std_ReturnType;

bool MesRequest(uint16_t did);
Std_ReturnType MesReadData(uint16_t did, uint8_t *data, uint16_t dataLen);
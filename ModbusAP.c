#include <stddef.h>
#include <string.h>
#include "ModbusAP.h"
#include "ModbusTCP.h"

#define APDU_MAX       253
#define FC_READ_COILS  0x01
#define FC_WRITE_COILS 0x0F
#define MAX_READ       2000
#define MAX_WRITE      1968
#define MAX_ADDR       65536

// Devolve 1 se os parametros sao validos, 0 caso contrario.
static int params_ok(char *server_add, uint32_t st_c, uint16_t n_c, uint8_t *val, uint16_t max)
{
    if (server_add == NULL || val == NULL) return 0;
    if (n_c < 1 || n_c > max) return 0;
    if (st_c < 1 || st_c + n_c - 1 > MAX_ADDR) return 0;
    return 1;
}

int Write_coils(char *server_add, int port, uint32_t st_c, uint16_t n_c, uint8_t *val)
{
    uint8_t APDU[APDU_MAX] = {0}, APDU_R[APDU_MAX] = {0};
    uint16_t addr, nbytes, i;

    // 1. validar parametros
    if (!params_ok(server_add, st_c, n_c, val, MAX_WRITE)) return MB_ERR_PARAM;

    // 2. montar o APDU
    addr = st_c - 1;            // na trama o endereco comeca em 0
    nbytes = (n_c + 7) / 8;

    APDU[0] = FC_WRITE_COILS;
    APDU[1] = addr >> 8;
    APDU[2] = addr & 0xFF;
    APDU[3] = n_c >> 8;
    APDU[4] = n_c & 0xFF;
    APDU[5] = nbytes;
    for (i = 0; i < n_c; i++)
        if (val[i]) APDU[6 + i / 8] |= 1 << (i % 8);

    // 3. enviar o pedido
    if (Send_Modbus_request(server_add, port, APDU, 6 + nbytes, APDU_R) < 0) return MB_ERR_COMM;

    // 4. verificar a resposta
    if (APDU_R[0] & 0x80) return APDU_R[1] ? -(int)APDU_R[1] : MB_ERR_RESP;
    if (memcmp(APDU_R, APDU, 5) != 0) return MB_ERR_RESP;

    // 5. sucesso
    return n_c;
}

int Read_coils(char *server_add, int port, uint32_t st_c, uint16_t n_c, uint8_t *val)
{
    uint8_t APDU[5], APDU_R[APDU_MAX] = {0};
    uint16_t addr, nbytes, i;

    // 1. validar parametros
    if (!params_ok(server_add, st_c, n_c, val, MAX_READ)) return MB_ERR_PARAM;

    // 2. montar o APDU
    addr = st_c - 1;
    nbytes = (n_c + 7) / 8;

    APDU[0] = FC_READ_COILS;
    APDU[1] = addr >> 8;
    APDU[2] = addr & 0xFF;
    APDU[3] = n_c >> 8;
    APDU[4] = n_c & 0xFF;

    // 3. enviar o pedido
    if (Send_Modbus_request(server_add, port, APDU, 5, APDU_R) < 0) return MB_ERR_COMM;

    // 4. verificar a resposta
    if (APDU_R[0] & 0x80) return APDU_R[1] ? -(int)APDU_R[1] : MB_ERR_RESP;
    if (APDU_R[0] != FC_READ_COILS || APDU_R[1] != nbytes) return MB_ERR_RESP;

    // 5. desempacotar as coils
    for (i = 0; i < n_c; i++)
        val[i] = (APDU_R[2 + i / 8] >> (i % 8)) & 1;

    return n_c;
}
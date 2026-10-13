#ifndef MODBUSAP_H
#define MODBUSAP_H
#include <stdint.h>

// Valores de erro (sempre < 0).
// Uma excecao do servidor devolve o codigo com sinal negativo (-1 a -11).
#define MB_ERR_PARAM (-100)  // parametros invalidos
#define MB_ERR_COMM  (-101)  // falha de comunicacao ou timeout
#define MB_ERR_RESP  (-102)  // resposta inesperada

// st_c: numero da primeira coil (1 a 65536)
// n_c:  quantidade de coils
// val:  uma coil por posicao (0 ou 1)
// Devolvem o numero de coils escritas/lidas, ou < 0 em erro.
int Write_coils(char *server_add, int port, uint32_t st_c, uint16_t n_c, uint8_t *val);
int Read_coils(char *server_add, int port, uint32_t st_c, uint16_t n_c, uint8_t *val);

#endif
#include <stdio.h>
#include "ModbusAP.h"

#define SERVER_ADDR "10.227.113.101"
#define SERVER_PORT 502

int main(void)
{
    uint8_t escrever[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    uint8_t lido[8];
    int r, i;

    r = Write_coils(SERVER_ADDR, SERVER_PORT, 1, 8, escrever);
    printf("Write_coils devolveu %d\n", r);

    r = Read_coils(SERVER_ADDR, SERVER_PORT, 1, 8, lido);
    printf("Read_coils devolveu %d: ", r);
    for (i = 0; i < 8; i++) printf("%d ", lido[i]);
    printf("\n");

    return 0;
}
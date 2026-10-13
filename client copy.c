#include <stdio.h>
#include "ModbusAP.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 502

int main(void)
{
    uint8_t val[10] = {1, 0, 1, 1, 0, 0, 1, 1, 1, 0};
    uint8_t lido[10];
    int r;

   // r = Write_coils(SERVER_ADDR, SERVER_PORT, 20, 10, val);
    //printf("Write_coils devolveu %d\n", r);

    r = Read_coils(SERVER_ADDR, SERVER_PORT, 20, 10, lido);
    printf("Read_coils devolveu %d: ", r);
    for (int i = 0; i < 10; i++) printf("%d ", lido[i]);
    printf("\n");

    return 0;
}
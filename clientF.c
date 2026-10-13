#include <stdio.h>
#include <unistd.h>
#include "ModbusTCP.h"
#include "ModbusAP.h"

#define SERVER_ADDR "127.0.0.1"   // muda para o IP do servidor
#define SERVER_PORT 502
#define OFFSET      5             // primeira coil (offset, comeca em 0)
#define N_COILS     5             // quantidade de coils

int main(void)
{
    uint8_t escrever[N_COILS] = {1, 0, 1, 1, 0};
    uint8_t dados[1] = {0};       // 5 coils cabem num byte
    uint8_t lido[1];
    int i, r;

    int sock = ModbusTCP_Connect(SERVER_ADDR, SERVER_PORT);
    if (sock < 0) {
        printf("Ligacao: %d\n", MB_ERR_COMM);
        return 1;
    }

    // empacotar as coils: coil i vai no bit i (o primeiro e o menos significativo)
    for (i = 0; i < N_COILS; i++)
        if (escrever[i]) dados[0] |= 1 << i;

    r = ModbusAP_WriteMultipleCoils_Req(sock, OFFSET, N_COILS, dados);
    printf("Escrita: %d\n", r);

    r = ModbusAP_ReadCoils_Req(sock, OFFSET, N_COILS, lido);
    printf("Leitura: %d", r);
    if (r > 0) {
        printf(" ->");
        for (i = 0; i < N_COILS; i++)
            printf(" %d", (lido[0] >> i) & 1);
    }
    printf("\n");

    close(sock);
    return 0;
}
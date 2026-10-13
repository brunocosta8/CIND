#include <stdio.h>
#include "ModbusAP.h"

#define SERVER1 "127.0.0.1"
#define SERVER2 "10.227.113.101"
#define PORT    502

// O enunciado usa offsets (a comecar em 0) e a biblioteca usa o numero
// da coil (a comecar em 1), por isso soma-se 1 a cada endereco.
#define COIL(offset) ((offset) + 1)

// Converte o retorno da biblioteca no resultado pedido pelo enunciado:
//   0 se correu bem, X > 0 se foi excecao Modbus, -1 nos outros erros.
static int resultado(int r)
{
    if (r >= 0) return 0;                                    // sucesso
    if (r == MB_ERR_PARAM || r == MB_ERR_COMM || r == MB_ERR_RESP)
        return -1;                                           // erro nao Modbus
    return -r;                                               // excecao: -X passa a X
}

int main(void)
{
    uint8_t W[8] = {1, 0, 1, 0, 0, 1, 1, 0};
    uint8_t A[10], B, C[10];
    int i, res;

    // 1. escrever W nas coils 120 a 127
    res = resultado(Write_coils(SERVER1, PORT, COIL(120), 8, W));

    // 2. ler as coils 130 a 139 para A
    if (res == 0)
        res = resultado(Read_coils(SERVER1, PORT, COIL(130), 10, A));

    // 3. ler a coil 140 para B
    if (res == 0)
        res = resultado(Read_coils(SERVER1, PORT, COIL(140), 1, &B));

    // calcular C
    if (res == 0)
        for (i = 0; i < 10; i++)
            C[i] = (B == 0) ? 1 : !A[i];

    // 4. escrever C nas coils 150 a 159 do servidor 1
    if (res == 0)
        res = resultado(Write_coils(SERVER1, PORT, COIL(150), 10, C));

    // 5. escrever C nas coils 160 a 169 do servidor 2
    if (res == 0)
        res = resultado(Write_coils(SERVER2, PORT, COIL(160), 10, C));

    // 6. resultado final
    printf("result = %d\n", res);
    return 0;
}
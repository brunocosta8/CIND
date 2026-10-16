#include <stdio.h>
#include <unistd.h>
#include "ModbusTCPF.h"
#include "ModbusAPF.h"

#define SERVER1 "127.0.0.1"
#define SERVER2 "127.0.0.1"     
#define PORT1   502
#define PORT2   502

static int resultado(int r)
{
    if (r >= 0) return 0;                       // correu bem          -> 0
    if (r == MB_ERR_PARAM || r == MB_ERR_COMM || r == MB_ERR_RESP)
        return -1;                              // -100/-101/-102      -> -1
    return -r;                                  // -2 -> 2, -1 -> 1, ...
}

int main(void)
{
    uint8_t escrever[4] = {1,1,0,1}; //escrever o que quisermos inicialmente
    uint8_t dados[1] = {0};    //buffer para meter o primeiros dados
    uint8_t lido[2];           //buffer para ler
    uint8_t A[10];             //array necessário
    uint8_t B=0;                //variavel que lemos
    uint8_t C[10];              //array para escrever
    uint8_t dados2[2] = {0};    //buffer para escrever a seguir
    int i, r;   
    int n_uns = 0; 
    uint8_t D=0;                //variavel para escrevr

    //ligação TCP
    int sock = ModbusTCP_Connect(SERVER1, PORT1);
    if (sock < 0) {
        printf("Resultado= %d\n", MB_ERR_COMM);
        return 1;
    }

    // empacotar as coils: coil i vai no bit i (o primeiro e o menos significativo)
    for (i = 0; i < 4; i++)
        if (escrever[i]) dados[0] |= 1 << i;

    //escrever as coils com 2 de offset e 4 coils
    r = resultado(ModbusAP_WriteMultipleCoils_Req(sock, 2, 4, dados));
    printf("Escrita: %d\n", r);

    //ler as coils com 6 de offset e 10 coils
    if(r==0)
    {
        r = resultado(ModbusAP_ReadCoils_Req(sock, 6, 10, lido));
        printf("Leitura: %d", r);
        if (r == 0) {
            printf(" ->");
            for (i = 0; i < 10; i++)
            {
                printf(" %d", (lido[i / 8] >> (i % 8)) & 1);
                A[i] = (lido[i / 8] >> (i % 8)) & 1; //passar o buufer lido para o array A

                /*A[i] = (lido[i / 8] >> (i % 8)) & 1;
                printf(" %d", A[i]);*/
            }
        }

        printf("\n");
    }

    //ler somente uma coil
    if(r==0)
    {
        r = resultado(ModbusAP_ReadCoils_Req(sock, 16, 1, lido));   
        printf("Leitura B: %d", r);
        if (r == 0) {
            B = lido[0] & 1;    //passar o buffer lido para a variavel B                          
            printf(" -> B = %d", B);
        }
        printf("\n");

        if(B==0)
        {
            for(int z=0;z<10;z++)
            {
                C[z]=A[z];
            }
        }else if(B==1)
        {
            for(int z=0;z<10;z++)
            {
                C[z]=!A[z];
            }
        }
    }

    //empacotar as coils no buffer dados2
    if(r==0)
    {
        for (i = 0; i < 10; i++)
            if (C[i]) dados2[i / 8] |= 1 << (i % 8);

        //escrever dados2 com offset 20 e 10 coils
        r = resultado(ModbusAP_WriteMultipleCoils_Req(sock, 20, 10, dados2));
        printf("Escrita: %d\n", r);

        
        for (i = 0; i < 10; i++) {
            n_uns += A[i];
        }

        if(n_uns>=5)
        {
            D=1;
        }
    }

    close(sock);                                       // fim do servidor 1

    // abrir servidor 2
    if (r == 0) {
        sock = ModbusTCP_Connect(SERVER2, PORT2);
        if (sock < 0) {
            r = -1;                                    // não ligou: erro não Modbus
        } else {
            r = resultado(ModbusAP_WriteMultipleCoils_Req(sock, 0, 1, &D));
            printf("Escrita D: %d\n", r);              // escrever 1 coil
            close(sock);
        }
    }

    printf("result = %d\n", r);
    return 0;

}
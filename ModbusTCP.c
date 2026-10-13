#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ModbusTCP.h"

#define MBAP_SIZE 7     // tamanho do cabecalho MBAP
#define APDU_MAX  253   // tamanho maximo de um APDU
#define UNIT_ID   1     // identificador da unidade (ver nota na conversa)
#define TIMEOUT_S 3     // tempo maximo de espera pela resposta, em segundos

// Envia exatamente len bytes. Devolve 0 se ok, -1 em erro.
static int send_all(int fd, uint8_t *buf, int len)
{
    int done = 0, n;

    while (done < len) {
        n = send(fd, buf + done, len - done, 0);
        if (n <= 0) return -1;
        done += n;
    }
    return 0;
}

// Recebe exatamente len bytes (o TCP pode entrega-los aos bocados).
// Devolve 0 se ok, -1 em erro, ligacao fechada ou timeout.
static int recv_all(int fd, uint8_t *buf, int len)
{
    int done = 0, n;

    while (done < len) {
        n = recv(fd, buf + done, len - done, 0);
        if (n <= 0) return -1;
        done += n;
    }
    return 0;
}

// Cria o socket e liga ao servidor. Devolve o descritor, ou -1 em erro.
static int connect_server(char *server_add, int port)
{
    struct sockaddr_in server;
    struct timeval tv = {TIMEOUT_S, 0};
    int fd;

    fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) return -1;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_aton(server_add, &server.sin_addr) == 0) {   // endereco IP mal formado
        close(fd);
        return -1;
    }

    // tempo limite para o envio e para a rececao
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int Send_Modbus_request(char *server_add, int port, uint8_t *APDU, int APDUlen, uint8_t *APDU_R)
{
    static uint16_t TI = 0;                  // identificador de transacao
    uint8_t PDU[MBAP_SIZE + APDU_MAX];       // pedido completo: MBAP + APDU
    uint8_t MBAP_R[MBAP_SIZE];               // cabecalho da resposta
    int fd, len, ret = -1;

    if (server_add == NULL || APDU == NULL || APDU_R == NULL) return -1;
    if (APDUlen < 1 || APDUlen > APDU_MAX) return -1;

    // 1. gerar o TI
    TI++;

    // 2. montar o PDU = MBAP + APDU
    PDU[0] = TI >> 8;                 // Transaction ID
    PDU[1] = TI & 0xFF;
    PDU[2] = 0;                       // Protocol ID = 0 (Modbus)
    PDU[3] = 0;
    PDU[4] = (APDUlen + 1) >> 8;      // Length = Unit ID + APDU
    PDU[5] = (APDUlen + 1) & 0xFF;
    PDU[6] = UNIT_ID;                 // Unit ID
    memcpy(PDU + MBAP_SIZE, APDU, APDUlen);

    // 3. abrir o socket e ligar ao servidor
    fd = connect_server(server_add, port);
    if (fd < 0) return -1;

    // 4. enviar o pedido
    if (send_all(fd, PDU, MBAP_SIZE + APDUlen) == 0 &&
        // 5. ler os 7 bytes do MBAP da resposta
        recv_all(fd, MBAP_R, MBAP_SIZE) == 0 &&
        // 6. o TI tem de ser o do pedido e o Protocol ID tem de ser 0
        MBAP_R[0] == PDU[0] && MBAP_R[1] == PDU[1] &&
        MBAP_R[2] == 0 && MBAP_R[3] == 0) {

        // 7. Length - 1 = tamanho do APDU de resposta
        len = ((MBAP_R[4] << 8) | MBAP_R[5]) - 1;

        // 8. ler o APDU de resposta
        if (len >= 1 && len <= APDU_MAX && recv_all(fd, APDU_R, len) == 0)
            ret = 0;
    }

    // 9. fechar o socket
    close(fd);
    return ret;
}
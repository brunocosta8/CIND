#include "ModbusTCP.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

static uint16_t last_tid;   // TI do ultimo pedido, para confirmar a resposta

// Instante atual, em segundos.
static double now_s(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1e6;
}

// Espera que o socket fique pronto (para ler ou para escrever) ate ao deadline.
// Devolve 1 se esta pronto, 0 se o tempo acabou, -1 em erro.
static int wait_fd(int sock, int for_write, double deadline) {
    fd_set set;
    struct timeval tv;
    double left = deadline - now_s();

    if (left <= 0) return 0;
    tv.tv_sec = (long)left;
    tv.tv_usec = (long)((left - tv.tv_sec) * 1e6);
    FD_ZERO(&set);
    FD_SET(sock, &set);
    return select(sock + 1, for_write ? NULL : &set, for_write ? &set : NULL, NULL, &tv);
}

// Le exatamente len bytes (o TCP pode entrega-los aos bocados), ate ao deadline.
static int recv_all(int sock, uint8_t *buf, int len, double deadline) {
    int done = 0, n;

    while (done < len) {
        if (wait_fd(sock, 0, deadline) <= 0) return -1;   // timeout
        n = recv(sock, buf + done, len - done, 0);
        if (n <= 0) return -1;                            // erro ou ligacao fechada
        done += n;
    }
    return 0;
}

int ModbusTCP_Connect(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server.sin_addr) != 1) {   // IP mal escrito
        close(sock);
        return -1;
    }

    // connect com tempo limite
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        int err = 0;
        socklen_t errlen = sizeof(err);
        if (errno != EINPROGRESS ||
            wait_fd(sock, 1, now_s() + MB_TIMEOUT_S) <= 0 ||
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0 || err != 0) {
            close(sock);
            return -1;
        }
    }
    fcntl(sock, F_SETFL, flags);
    return sock;
}

int ModbusTCP_Send_Req(int sock, uint16_t tid, uint8_t unit_id, const uint8_t *pdu, uint16_t pdu_len) {
    uint8_t frame[260];
    uint16_t length = pdu_len + 1; // Tamanho do PDU + 1 byte do Unit ID

    if (pdu_len < 1 || pdu_len > 253) return -1;   // tamanho maximo de um PDU Modbus

    // Construção do MBAP Header (7 bytes em Big Endian)
    frame[0] = (tid >> 8) & 0xFF; // Transaction ID (High)
    frame[1] = tid & 0xFF;        // Transaction ID (Low)
    frame[2] = 0x00;              // Protocol ID (sempre 0 para Modbus)
    frame[3] = 0x00;
    frame[4] = (length >> 8) & 0xFF; // Length (High)
    frame[5] = length & 0xFF;        // Length (Low)
    frame[6] = unit_id;           // Unit ID

    // Cópia do PDU (Function Code + Dados) para a trama principal
    memcpy(&frame[7], pdu, pdu_len);

    int total_len = 7 + pdu_len;
    last_tid = tid;

    // Envio para a rede (pode ser preciso mais de um send)
    int done = 0;
    while (done < total_len) {
        int n = send(sock, frame + done, total_len - done, 0);
        if (n <= 0) return -1;
        done += n;
    }
    return total_len;
}

int ModbusTCP_Receive_Conf(int sock, uint8_t *pdu_out) {
    uint8_t header[7];
    double deadline = now_s() + MB_TIMEOUT_S;   // limite para toda a resposta

    // Leitura obrigatória dos 7 bytes do MBAP primeiro
    if (recv_all(sock, header, 7, deadline) < 0) return -1;

    // O TI tem de ser o do pedido e o Protocol ID tem de ser 0
    if (((header[0] << 8) | header[1]) != last_tid) return -1;
    if (header[2] != 0 || header[3] != 0) return -1;

    // Extração do tamanho do PDU a partir do MBAP
    uint16_t length = (header[4] << 8) | header[5];
    if (length < 2 || length > 254) return -1;   // PDU entre 1 e 253 bytes
    uint16_t pdu_len = length - 1; // Retira o byte do Unit ID

    // Leitura do PDU restante com o tamanho exato calculado
    if (recv_all(sock, pdu_out, pdu_len, deadline) < 0) return -1;

    return pdu_len;
}
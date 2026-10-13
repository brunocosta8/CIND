#ifndef MODBUSTCP_H
#define MODBUSTCP_H

#include <stdint.h>

#define MB_TIMEOUT_S 3   // tempo maximo para ligar e para receber a resposta (segundos)

int ModbusTCP_Connect(const char *ip, int port);
int ModbusTCP_Send_Req(int sock, uint16_t tid, uint8_t unit_id, const uint8_t *pdu, uint16_t pdu_len);
int ModbusTCP_Receive_Conf(int sock, uint8_t *pdu_out);

#endif
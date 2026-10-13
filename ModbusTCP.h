#ifndef MODBUSTCP_H
#define MODBUSTCP_H
#include <stdint.h>

int Send_Modbus_request(char *server_add, int port, uint8_t *APDU, int APDUlen, uint8_t *APDU_R);

#endif
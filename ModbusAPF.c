#include "ModbusAPF.h"
#include "ModbusTCPF.h"
#include <stdio.h>
#include <string.h>

static uint16_t current_tid = 1;


int ModbusAP_ReadCoils_Req(int sock, uint16_t start_addr, uint16_t quantity, uint8_t *coils_status_out) {
    uint8_t req_pdu[5];
    uint8_t resp_pdu[256];
    uint8_t expected_bytes = (quantity + 7) / 8;

    if (quantity < 1 || quantity > 2000) return MB_ERR_PARAM;          // limites da norma
    if ((uint32_t)start_addr + quantity > 65536) return MB_ERR_PARAM;

    req_pdu[0] = 0x01; 
    req_pdu[1] = (start_addr >> 8) & 0xFF; 
    req_pdu[2] = start_addr & 0xFF;        
    req_pdu[3] = (quantity >> 8) & 0xFF;   
    req_pdu[4] = quantity & 0xFF;          

    if (ModbusTCP_Send_Req(sock, current_tid++, UNIT_ID, req_pdu, 5) < 0) return MB_ERR_COMM;

    int resp_len = ModbusTCP_Receive_Conf(sock, resp_pdu);
    if (resp_len < 0) return MB_ERR_COMM;                              // sem resposta / timeout

    if (resp_pdu[0] == (0x01 | 0x80))                                  // excecao: -codigo
        return (resp_len >= 2) ? -resp_pdu[1] : MB_ERR_RESP;

    if (resp_pdu[0] != 0x01 || resp_pdu[1] != expected_bytes ||        // resposta tem de bater certo
        resp_len != 2 + expected_bytes) return MB_ERR_RESP;

    uint8_t byte_count = resp_pdu[1];
    memcpy(coils_status_out, &resp_pdu[2], byte_count);

    return byte_count; 
}

int ModbusAP_WriteMultipleCoils_Req(int sock, uint16_t start_addr, uint16_t quantity, const uint8_t *coils_data) {
    uint8_t req_pdu[256];
    uint8_t resp_pdu[256];

    if (quantity < 1 || quantity > 1968) return MB_ERR_PARAM;          // limites da norma
    if ((uint32_t)start_addr + quantity > 65536) return MB_ERR_PARAM;

    uint8_t byte_count = (quantity % 8 == 0) ? (quantity / 8) : (quantity / 8) + 1;

    req_pdu[0] = 0x0F; 
    req_pdu[1] = (start_addr >> 8) & 0xFF;
    req_pdu[2] = start_addr & 0xFF;
    req_pdu[3] = (quantity >> 8) & 0xFF;
    req_pdu[4] = quantity & 0xFF;
    req_pdu[5] = byte_count;
    memcpy(&req_pdu[6], coils_data, byte_count); 

    if (ModbusTCP_Send_Req(sock, current_tid++, UNIT_ID, req_pdu, 6 + byte_count) < 0) return MB_ERR_COMM;

    int resp_len = ModbusTCP_Receive_Conf(sock, resp_pdu);
    if (resp_len < 0) return MB_ERR_COMM;                              // sem resposta / timeout

    if (resp_pdu[0] == (0x0F | 0x80))                                  // excecao: -codigo
        return (resp_len >= 2) ? -resp_pdu[1] : MB_ERR_RESP;

    if (resp_len != 5 || memcmp(resp_pdu, req_pdu, 5) != 0)           // tem de repetir fc, endereco e quantidade
        return MB_ERR_RESP;

    return 0; 
}
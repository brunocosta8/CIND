#ifndef MODBUSAP_H
#define MODBUSAP_H
#include <stdint.h>
 
#define UNIT_ID   7          // Unit ID do servidor: muda aqui quando precisares
 
// Codigos de erro (sempre negativos)
//   -1 a -11 : excecao Modbus enviada pelo servidor (-codigo da excecao)
#define MB_ERR_PARAM (-100)   // parametros invalidos (o pedido nem e enviado)
#define MB_ERR_COMM  (-101)   // sem ligacao, falha no envio, sem resposta ou timeout
#define MB_ERR_RESP  (-102)   // resposta que nao bate certo com o pedido
 
// Sucesso: a leitura devolve o numero de bytes lidos, a escrita devolve 0
int ModbusAP_ReadCoils_Req(int sock, uint16_t start_addr, uint16_t quantity, uint8_t *coils_status_out);
int ModbusAP_WriteMultipleCoils_Req(int sock, uint16_t start_addr, uint16_t quantity, const uint8_t *coils_data);
 
#endif
 
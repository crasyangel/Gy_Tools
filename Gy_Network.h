#ifndef _GY_NETWORK_H_
#define _GY_NETWORK_H_


#ifdef  __cplusplus
extern "C" {
#endif

#include "Gy_Common.h"


int Ping(const char* adress, const uint16_t port, const uint8_t retry);
int GetWiredIPAddress(char *ipaddr, uint32_t ipaddr_len);
uint32_t GetRand(uint32_t seed_me, uint32_t range);


#ifdef  __cplusplus
}
#endif

#endif //_GY_NETWORK_H_

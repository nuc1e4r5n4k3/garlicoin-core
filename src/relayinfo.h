#ifndef BITCOIN_RELAYINFO_H
#define BITCOIN_RELAYINFO_H

#include "uint256.h"
#include "net.h"


void relayinfo_register_new_object(const uint256 &object, const CNetAddr &source, int type = UNDEFINED);
void relayinfo_register_new_object(const CInv &object, const CNode &source);
std::string relayinfo_get_source_for(const uint256 &object);

#endif




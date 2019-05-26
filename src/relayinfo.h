#ifndef BITCOIN_RELAYINFO_H
#define BITCOIN_RELAYINFO_H

#include "uint256.h"
#include "net.h"

class CRelayInfo
{
    public:
        CRelayInfo(const uint256 &object = uint256(), const std::string &source = "", time_t receivedAt = 0);

        inline bool hasInfo(void) const
        {
            return !this->source().empty();
        }

        inline const uint256 &object(void) const
        {
            return _object;
        }

        inline const std::string &source(void) const
        {
            return _source;
        }

        inline const time_t &timestamp(void) const
        {
            return _timestamp;
        }

    private:
        uint256 _object;
        std::string _source;
        time_t _timestamp;
};


void relayinfo_register_new_object(const uint256 &object, const CNetAddr &source, int type = UNDEFINED);
void relayinfo_register_new_object(const CInv &object, const CNode &source);

CRelayInfo relayinfo_get_info_for(const uint256 &object);
std::string relayinfo_get_source_for(const uint256 &object);

#endif




#include "relayinfo.h"

#include "sync.h"   /* For CCriticalSection */

#include <time.h>


#define RINGBUF_SIZE            256

#define RINGBUF_PREV(src)       (((src) + RINGBUF_SIZE - 1) % RINGBUF_SIZE)
#define RINGBUF_NEXT(src)       (((src) + 1) % RINGBUF_SIZE)


CRelayInfo::CRelayInfo(const uint256 &object, const std::string &source, time_t receivedAt) :
    _object(object),
    _source(source),
    _timestamp(receivedAt)
{
    if (!_timestamp && !source.empty())
        time(&_timestamp);
}


static CCriticalSection Lock;
static CRelayInfo RingBuffer[RINGBUF_SIZE];
static int lastWrite = -1;


static bool _is_public(const CNetAddr &addr)
{
    if (addr.IsIPv4())
    {
        return !(
            addr.IsRFC1918() ||
            addr.IsRFC2544() ||
            addr.IsRFC3927() ||
            addr.IsRFC6598()
        );
    }
    else if (addr.IsIPv6())
    {
        return !(
            addr.IsRFC4193() ||
            addr.IsRFC4862()
        );
    }

    /* We are probably not interested in anything else */
    return false;
}

static const CRelayInfo &_relayinfo_get_info_for_nolock(const uint256 &object)
{
    static const CRelayInfo NoInfo = CRelayInfo();

    /* Sanity check */
    if (lastWrite < 0 || lastWrite >= RINGBUF_SIZE)
        return NoInfo;

    int idx = lastWrite; do
    {
        const CRelayInfo &info = RingBuffer[idx];
        idx = RINGBUF_PREV(idx);

        if (info.object() == object)
            return info;
    }
    while (idx != lastWrite);

    return NoInfo;
}

static bool _relayinfo_register_new_object(const uint256 &object, const std::string &source)
{
    LOCK(Lock);

    if (_relayinfo_get_info_for_nolock(object).hasInfo())
        return false;

    lastWrite = RINGBUF_NEXT(lastWrite);

    RingBuffer[lastWrite] = CRelayInfo(object, source);
    return true;
}

void relayinfo_register_new_object(const uint256 &object, const CNetAddr &source, int type)
{
    if (type && type != MSG_BLOCK && type != MSG_TX)
        return;

    if (!_is_public(source))
        return;

    std::string sourceIp = source.ToStringIP();

    bool inserted = _relayinfo_register_new_object(object, sourceIp);

    if (!type)
        return;     /* No idea what it is, so do not log */

    LogPrintf("Received %s %s from %s%s\n", type == MSG_TX ? "tx " : "blk", object.ToString(), sourceIp, inserted ? " [FIRST]" : "");
}

void relayinfo_register_new_object(const CInv &object, const CNode &source)
{
    relayinfo_register_new_object(object.hash, source.addr, object.type);
}

CRelayInfo relayinfo_get_info_for(const uint256 &object)
{
    LOCK(Lock);
    return _relayinfo_get_info_for_nolock(object);
}

std::string relayinfo_get_source_for(const uint256 &object)
{
    LOCK(Lock);
    return _relayinfo_get_info_for_nolock(object).source();
}


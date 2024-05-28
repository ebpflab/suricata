
#include "suricata-common.h"
#include "suricata.h"
#include "packet.h"

#include "threads.h"
#include "threadvars.h"
#include "tm-threads.h"
#include "tm-threads-common.h"
#include "util-runmodes.h"
#include "tmqh-packetpool.h"
#include "source-isolated.h"


TmEcode ReceiveIsolated(ThreadVars *tv, const uint8_t *pkt, int pktlen, uint8_t proto)
{
	/* make sure we have at least one packet in the packet pool, to prevent
	 * us from alloc'ing packets at line rate */
	//PacketPoolWait();
	
	Packet *p = PacketGetFromQueueOrAlloc();
	
    if (unlikely(p == NULL)) {
        SCReturnInt(TM_ECODE_FAILED);
    }
	
	PacketSetData(p, pkt, pktlen);
    SCLogDebug("pktlen: %" PRIu32 " (pkt %p, pkt data %p)", GET_PKT_LEN(p), p, GET_PKT_DATA(p));
	
	p->datalink = proto;
    PKT_SET_SRC(p, PKT_SRC_ISOLATED);
	
    if (TmThreadsSlotProcessPkt(tv, tv->tm_slots, p) != TM_ECODE_OK) {
		SCReturnInt(TM_ECODE_FAILED);
    }

	SCReturnInt(TM_ECODE_OK);

}


static TmEcode DecodeIsolated(ThreadVars *tv, Packet *p, void *data)
{
    SCEnter();
    DecodeThreadVars *dtv = (DecodeThreadVars *)data;

    BUG_ON(PKT_IS_PSEUDOPKT(p));

    /* update counters */
    DecodeUpdatePacketCounters(tv, dtv, p);

    /* If suri has set vlan during reading, we increase vlan counter */
    if (p->vlan_idx) {
        StatsIncr(tv, dtv->counter_vlan);
    }

    /* call the decoder */
    DecodeLinkLayer(tv, dtv, p->datalink, p, GET_PKT_DATA(p), GET_PKT_LEN(p));

    PacketDecodeFinalize(tv, dtv, p);

    SCReturnInt(TM_ECODE_OK);
}


static TmEcode DecodeIsolatedThreadInit(ThreadVars *tv, const void *initdata, void **data)
{
    SCEnter();
    DecodeThreadVars *dtv = NULL;

    dtv = DecodeThreadVarsAlloc(tv);

    if (dtv == NULL)
        SCReturnInt(TM_ECODE_FAILED);

    DecodeRegisterPerfCounters(dtv, tv);

    *data = (void *)dtv;
	
	//(void)SC_ATOMIC_SET(tv->tm_slots->slot_data, dtv);

    SCReturnInt(TM_ECODE_OK);
}

static TmEcode DecodeIsolatedThreadDeinit(ThreadVars *tv, void *data)
{
    SCEnter();
    if (data != NULL)
        DecodeThreadVarsFree(tv, data);
    SCReturnInt(TM_ECODE_OK);
}

void TmModuleDecodeIsolatedRegister(void)
{
    tmm_modules[TMM_DECODEISOLATED].name = "DecodeISOLATED";
    tmm_modules[TMM_DECODEISOLATED].ThreadInit = DecodeIsolatedThreadInit;
    tmm_modules[TMM_DECODEISOLATED].Func = DecodeIsolated;
    tmm_modules[TMM_DECODEISOLATED].ThreadExitPrintStats = NULL;
    tmm_modules[TMM_DECODEISOLATED].ThreadDeinit = DecodeIsolatedThreadDeinit;
    tmm_modules[TMM_DECODEISOLATED].cap_flags = 0;
    tmm_modules[TMM_DECODEISOLATED].flags = TM_FLAG_DECODE_TM;
}

/* level 为包所在的层 */

int ReceivePkt(uint8_t level, const uint8_t *pkt, int pktlen)
{
	ThreadVars *tv = RunModeIsolatedThreadVars();
	TmEcode ret = -1;
	
	if (!tv) {
		return ret;
	}
	
	ret = ReceiveIsolated(tv, pkt, pktlen, level);
	if (ret == TM_ECODE_OK) {
		ret = 0;
	}
	
	return ret;
}


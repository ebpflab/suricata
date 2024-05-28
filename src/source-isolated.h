#ifndef __SOURCE_ISOLATED_H__
#define __SOURCE_ISOLATED_H__

typedef struct IsolatedConfig_
{
    char iface[32];
    /* number of threads */
    int threads;
    uint8_t copy_mode;
    const char *out_iface;
} IsolatedConfig;

void TmModuleDecodeIsolatedRegister(void);
int ReceivePkt(uint8_t level, const uint8_t *pkt, int pktlen);

#endif /* __SOURCE_ISOLATED_H__ */


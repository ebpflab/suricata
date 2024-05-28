#ifndef __EDPI_H__
#define __EDPI_H__

int EdpiInit(char *confpath);
void EdpiExit(void);
int EdpiReceivePkt(uint8_t level, const uint8_t *pkt, int pktlen);

#endif /* __EDPI_H__ */


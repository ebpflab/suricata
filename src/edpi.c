
#include "suricata-common.h"
#include "suricata.h"
#include "threads.h"
#include "source-isolated.h"
#include "edpi.h"


void *SuricataThread(void *argv)
{
	SuricataMain(4, argv);

	return NULL;
}

int EdpiInit(char *confpath)
{
	char *argv[5];
	argv[0] = "libipds";
	argv[1] = "-c";
	argv[2] = confpath;
	argv[3] = "--isolated";
	argv[4] = NULL;

	int status;
	pthread_t pthread;
    status = pthread_create(&pthread, NULL, SuricataThread, argv);

	while(!SuricatRuning());
	
	return 0;
}

void EdpiExit(void)
{
	EngineStop();
}


int EdpiReceivePkt(uint8_t level, const uint8_t *pkt, int pktlen)
{
	return ReceivePkt(level, pkt, pktlen);
}




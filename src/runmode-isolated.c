
#include "suricata-common.h"
#include "suricata.h"
#include "tm-threads.h"
#include "runmodes.h"
#include "util-runmodes.h"

#include "runmode-isolated.h"
#include "source-isolated.h"

const char *RunModeIsolatedGetDefaultMode(void)
{
    return "workers";
}

static int IsolatedRunModeIsIPS(void)
{
	return 0;
}

static void IsolatedRunModeEnableIPS(void)
{
    if (IsolatedRunModeIsIPS()) {
        SCLogInfo("Setting IPS mode");
        EngineModeSetIPS();
    }
}

static void *ParseIsolatedConfigure(const char *iface)
{
    IsolatedConfig *iconf = SCCalloc(1, sizeof(*iconf));
    if (iconf == NULL) {
        FatalError("ISOLATED configuration could not be parsed");
    }
	//TODO isolated parse

    return iconf;
}


int RunModeIdsIsolatedWorkers(void)
{
    SCEnter();
	
    int ret;
    TimeModeSetLive();

    ret = RunModeSetIsolatedWorkers(ParseIsolatedConfigure, NULL,
            "ReceiveISOLATED", "DecodeISOLATED", thread_name_workers, "");
    if (ret != 0) {
        FatalError("Unable to start runmode");
    }

    SCLogDebug("RunModeIdsIsolatedWorkers initialised");
	
    SCReturnInt(0);
}


void RunModeIsolatedRegister(void)
{
    RunModeRegisterNewRunMode(RUNMODE_ISOLATED, "workers",
            "Workers ISOLATED mode, each thread does all"
            " tasks from acquisition to logging",
            RunModeIdsIsolatedWorkers, IsolatedRunModeEnableIPS);
}


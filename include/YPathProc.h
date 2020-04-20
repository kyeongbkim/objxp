#ifndef __YPATH_PROC_H__
#define __YPATH_PROC_H__

#include <YPath.h>

/************************************************************/
/*                                                          */
/* - See YPath.h for naming rules                           */
/* - Keep this file sorted by macro value not by macro name */
/*                                                          */
/************************************************************/

#define YpProcSession           "/proc/session"
#define YpProcSession__SID      "/proc/session/${SID}"
#define YpProcSession__SID__Cmd "/proc/session/${SID}/cmd"
#define YpProcSession__SID__Rsp "/proc/session/${SID}/rsp"
#define YpProcLocalSubscription "/proc/local/subscription"
#define YpProcLogTransaction    "/proc/log/transaction"

#endif


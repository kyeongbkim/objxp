#ifndef __YPATH_SERVICE_H__
#define __YPATH_SERVICE_H__

#include <YPath.h>

/************************************************************/
/*                                                          */
/* - See YPath.h for naming rules                           */
/* - Keep this file sorted by macro value not by macro name */
/*                                                          */
/************************************************************/

#define YpMgmtRed                   "/app/mgmtRed"
#define YpMgmtRed__LOCATION__VAR    "/app/mgmtRed/${LOCATION=local}/${VAR}"

#define YpPon                       "/app/ypon"
#define YpPon__Twdm                 YpPon"/twdm"
#define YpPon__TWDM                 YpPon"/twdm/${TWDM}"
#define YpPon__TWDM__Config         YpPon"/twdm/${TWDM}/config"
#define YpPon__TWDM__Alloc_Id       YpPon"/twdm/${TWDM}/alloc_id"
#define YpPon__TWDM__Alloc_IdAllocated     \
        YpPon"/twdm/${TWDM}/alloc_id/allocated"
#define YpPon__TWDM__Alloc_IdAllocated__ID \
        YpPon"/twdm/${TWDM}/alloc_id/allocated/${ID}"
#define YpPon__TWDM__Alloc_IdStatus        \
        YpPon"/twdm/${TWDM}/alloc_id/status"
#define YpPon__TWDM__Alloc_IdStatus__ID    \
        YpPon"/twdm/${TWDM}/alloc_id/status/${ID}"

#define YpPon__Olt                  YpPon"/olt"
#define YpPon__OLT                  YpPon"/olt/${OLT}"
#define YpPon__OLT__Onu             YpPon"/olt/${OLT}/onu"
#define YpPon__OLT__ONU             YpPon"/olt/${OLT}/onu/${ONU}"

#define YpOnuBase                        YpPon__OLT__ONU
#define YpOnuBaseTrafficAlloc_IdReq      YpOnuBase"/traffic/alloc_id/req"
#define YpOnuBaseTrafficAlloc_IdReq__ID  YpOnuBase"/traffic/alloc_id/req/${ID}"
#define YpOnuBaseTrafficAlloc_IdRsp      YpOnuBase"/traffic/alloc_id/rsp"
#define YpOnuBaseTrafficAlloc_IdRsp__ID  YpOnuBase"/traffic/alloc_id/rsp/${ID}"

#define YpRtgAttReq         "/service/rtg/att/req"
#define YpRtgAttReq__ID     "/service/rtg/att/req/${ID}"
#define YpRtgAttRsp         "/service/rtg/att/rsp"
#define YpRtgAttRsp__ID     "/service/rtg/att/rsp/${ID}"
#define YpRtgAttStatus      "/service/rtg/att/status"
#define YpRtgAttStatus__DUT "/service/rtg/att/status/${DUT}"

#define YpRtgTopo__DUT__Intf "/service/rtg/topo/dut/${DUT}/intf"
#define YpRtgTopo__DUT__INTF "/service/rtg/topo/dut/${DUT}/intf/${INTF}"
#define YpRtgTopo__HOST__VwireIntf \
                             "/service/rtg/topo/host/${HOST}/vwire/intf"
#define YpRtgTopo__HOST__VwireIntf__INTF \
                             "/service/rtg/topo/host/${HOST}/vwire/intf/${INTF}"



#endif


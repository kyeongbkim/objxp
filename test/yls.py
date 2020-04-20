#!/usr/bin/env python

from __future__ import print_function
import pdb
import sys
import getopt
import json
import objxp
from objxp import DynamicCast_ObexJsonObject
from ObjXp import *

agentName = "yls.py"
cmdId = "yls.py"


def main(argv):
    usage = 'yls.py -s <server> <path>'
    svrInfo = None
    lsPath = None

    try:
        opts, remainder = getopt.getopt(argv, "hs:", ["server="])
    except getopt.GetoptError:
        print(usage)
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print(usage)
            sys.exit()
        elif opt in ("-s", "--svr"):
            svrInfo = arg

    if svrInfo == None or remainder == []:
        print(usage)
        sys.exit()

    lsPath = remainder[0].rstrip('/')
    if not lsPath:
        lsPath = '/'

    SetTraceLevel(8)

    ioSvc = CreateObject(objxp.BasicIoService)
    cbRcv = CreateObject(objxp.ObexCallbackReceiver, agentName)
    ccb = CreateObject(objxp.DefaultConnectionCallback)
    oc = CreateObject(objxp.ObexClient, agentName, ioSvc, ccb)
    oc.connect(agentName, svrInfo, 3000, 0)

    cs = WaitUntil(ioSvc, lambda: oc.getSession(agentName), period=10)
    assert cs
    assert WaitUntil(ioSvc, lambda: cs.getPeerName(), period=10)

    cmdPath = "/proc/session/%s/cmd" % cs.getPeerName()
    rspPath = "/proc/session/%s/rsp" % cs.getPeerName()

    cs.subscribe(rspPath)
    cs.registerCallback(rspPath, cbRcv)

    # pdb.set_trace()

    cmd = '{"id":"%s", "cmd":"%s"}' % (cmdId, "ls " + lsPath)
    jCmdObj = CreateObject(objxp.ObexJsonObject, cmd)
    cs.putObject(cmdPath, jCmdObj)

    ret = WaitUntil(ioSvc, lambda: cbRcv.count() > 0, period=10)
    assert ret

    cbTuple = cbRcv.pop()
    assert cbTuple.path() == rspPath
    jRspObj = DynamicCast_ObexJsonObject(cbTuple.obj())
    assert jRspObj
    rsp = json.loads(jRspObj.getString())
    assert rsp['id'] == cmdId
    if 'err' in rsp:
        print("Error:", rsp['err'])
        sys.exit()

    entries = rsp['rsp']
    if len(entries) == 0:
        print('<empty>')
    else:
        for e in entries:
            print(e)


if __name__ == "__main__":
    main(sys.argv[1:])
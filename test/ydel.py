#!/usr/bin/env python

from __future__ import print_function
import pdb
import sys
import getopt
import json
import objxp
from objxp import DynamicCast_ObexJsonObject
from ObjXp import *

agentName = "ydel.py"
cmdId = "ydel.py"


def main(argv):
    usage = 'ydel.py -s <server> <path>'
    svrInfo = None
    delPath = None

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

    delPath = remainder[0].rstrip('/')
    if not delPath:
        print('Invalid path')
        sys.exit()

    SetTraceLevel(8)

    ioSvc = CreateObject(objxp.BasicIoService)
    ccb = CreateObject(objxp.DefaultConnectionCallback)
    oc = CreateObject(objxp.ObexClient, agentName, ioSvc, ccb)
    oc.connect(agentName, svrInfo, 3000, 0)

    cs = WaitUntil(ioSvc, lambda: oc.getSession(agentName), period=10)
    assert cs
    assert WaitUntil(ioSvc, lambda: cs.getPeerName(), period=10)

    cs.delObject(delPath)
    ioSvc.run(500)

if __name__ == "__main__":
    main(sys.argv[1:])

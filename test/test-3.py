#!/usr/bin/env python

from __future__ import print_function
import pdb
import objxp
from ObjXp import *


def printSessionSummaries(s):
    ss = s.getSessionSummaries()
    for k in ss.keys():
        print(k, ss[k])


serverName = "ObexTestServer"
sockName = "sock." + serverName
agentName = "ObexTestClient"

pdb.set_trace()
ioSvc = CreateObject(objxp.BasicIoService)
ccb = CreateObject(objxp.DefaultConnectionCallback)
os = CreateObject(objxp.ObexServer, serverName, sockName, 0, ioSvc)
oc = CreateObject(objxp.ObexClient, agentName, ioSvc, ccb)
oc.connect(agentName, sockName, 3000, 0)
oc.connect("HelloClient", sockName, 3000, 0)
printSessionSummaries(oc)
ioSvc.run(100)
printSessionSummaries(oc)
printSessionSummaries(os)
cs = oc.getSession("HelloClient")
blob = CreateObject(objxp.ObexStringObject, "0123456789" * 7000)
obj = CreateObject(objxp.ObexStringObject, "0123456789")
cs.putObject('/a/b/c', blob)
cs.putObject('/a/b/d', obj);
cs.putObject('/a/b/e', blob)
cs.subscribe('/a/*');
ioSvc.run(3000)

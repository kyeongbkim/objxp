#!/usr/bin/env python

from __future__ import print_function
import pdb
import objxp
from objxp import DynamicCast_ObexStringObject
from ObjXp import *


def printSessionSummaries(s):
    ss = s.getSessionSummaries()
    for k in ss.keys():
        print(k, ss[k])


SetTraceLevel(8)

serverName = "ObexTestServer"
sockName = "sock." + serverName
agentName = "ObexTestClient"

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
oc.disconnect("HelloClient")
oc.disconnect(agentName)
ioSvc.run(100)
printSessionSummaries(oc)
ioSvc.run(1000)

oTree = CreateObject(objxp.ObexTree)
oCb1 = CreateObject(objxp.ObexCallbackReceiver, "Cb1")
oObj1 = CreateObject(objxp.ObexStringObject, "Hello")
oObj2 = CreateObject(objxp.ObexStringObject, "World")
oTree.registerCallback("/hardware/portInfo/et1", oCb1)
oTree.putObject("/hardware/portInfo/et1", oObj1)
oTree.putObject("/hardware/portInfo/et2", oObj2)
assert oCb1.count() == 1
cbTuple = oCb1.pop()
assert cbTuple
assert cbTuple.path() == "/hardware/portInfo/et1"
assert DynamicCast_ObexStringObject(cbTuple.obj()).equals(oObj1)

ioSvc.run(1000)

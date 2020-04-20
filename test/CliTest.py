#!/usr/bin/env python

from __future__ import print_function
import sys, pdb
import objxp
from ObjXp import *


def printSessionSummaries(s):
    ss = s.getSessionSummaries()
    for k in ss.keys():
        print(k, ss[k])


argc = len(sys.argv)
argv = sys.argv

assert argc > 1, "Client name is empty"
cliName = argv[1]
sessionName = cliName
svrName = "SvrTest" if argc < 3 else argv[2]
sockName = "sock." + svrName

ioSvc = BasicIoService()
ccb = CreateObject(objxp.DefaultConnectionCallback)
oc = CreateObject(objxp.ObexClient, cliName, ioSvc.getIoService(), ccb)
oc.connect(sessionName, sockName, 0, 0)
ioSvc.run(100)
printSessionSummaries(oc)

cs = oc.getSession(sessionName)
ioSvc.run(100)
printSessionSummaries(oc)

if sessionName == "c1":
    cs.subscribe('/*');
    while True:
        ioSvc.run()
        pdb.set_trace()

pdb.set_trace()
cs.subscribe('/*');
obj1 = CreateObject(objxp.ObexStringObject, "HelloWorld")
obj2 = CreateObject(objxp.ObexStringObject, "0123456789" * 1000)
cs.putObject('/a/b/c', obj1)

while True:
    ioSvc.run()
    pdb.set_trace()

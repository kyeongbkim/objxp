#!/usr/bin/env python

import sys, pdb
import objxp
from ObjXp import *

argc = len(sys.argv)
argv = sys.argv

svrName = "SvrTest" if argc < 2 else argv[1]
sockName = "sock." + svrName

ioSvc = BasicIoService()
os = CreateObject(objxp.ObexServer, svrName, sockName, 0, ioSvc.getIoService())

loop = True
while loop:
    ioSvc.run()
    pdb.set_trace()

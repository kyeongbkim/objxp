#!/usr/bin/env python

from __future__ import print_function
import getopt, sys, os, pdb
import objxp
from ObjXp import *

SetTraceLevel(8)


def usage(progName):
    print('Usage:', progName, '-s <SERVER> <OBJ_PATH> <OBJ_STRING>')
    print('       SERVER: "192.168.x.x:5555" or "sockName"')
    sys.exit(2)


def main():
    progName = os.path.basename(sys.argv[0]);
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hs:", ["help", "server="])
    except getopt.GetoptError as err:
        # print help information and exit:
        print(str(err))  # will print something like "option -a not recognized"
        usage(progName)

    server = None
    objPath = None
    objStr = None

    for o, a in opts:
        if o in ("-h", "--help"):
            usage(progName)
        elif o in ("-s", "--server"):
            server = a
        else:
            assert False, "unhandled option"

    if server == None:
        usage(progName)

    if ":" in server:
        (svrAddr, svrPort) = server.split(':')
    else:
        svrSock = server

    if len(args) < 2:
        usage(progName)

    objPath = args[0]
    objStr = args[1]

    ioSvc = CreateObject(objxp.BasicIoService)
    ccb = CreateObject(objxp.DefaultConnectionCallback)
    oc = CreateObject(objxp.ObexClient, progName, ioSvc, ccb)
    oc.connect(progName, server, 0, 0)
    ioSvc.run(100)

    cs = oc.getSession(progName)
    assert (cs)

    obj = CreateObject(objxp.ObexStringObject, objStr)
    cs.putObject(objPath, obj)
    ioSvc.run(100)


if __name__ == "__main__":
    main()

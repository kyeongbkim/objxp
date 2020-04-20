#!/usr/bin/env python

from __future__ import print_function
import pdb
import sys
import getopt
import json
import curses
import objxp
from objxp import DynamicCast_ObexJsonObject
from ObjXp import *

agentName = "ytree.py"
cmdId = "ytree.py"


def main(argv):
    usage = 'ytree.py -s <server>'
    svrInfo = None
    catPath = None

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

    if svrInfo == None:
        print(usage)
        sys.exit()

    SetTraceLevel(0)

    ioSvc = CreateObject(objxp.BasicIoService)
    cbRcv = CreateObject(objxp.ObexCallbackReceiver, agentName)
    ccb = CreateObject(objxp.DefaultConnectionCallback)
    oc = CreateObject(objxp.ObexClient, agentName, ioSvc, ccb)
    oc.connect(agentName, svrInfo, 3000, 0)
    cs = WaitUntil(ioSvc, lambda: oc.getSession(agentName), period=10)
    assert cs
    assert WaitUntil(ioSvc, lambda: cs.getPeerName(), period=10)
    cs.subscribe("/*")

    cs.registerCallback("/*", cbRcv)
    log = []
    # pdb.set_trace()
    try:
        stdscr = curses.initscr()
        stdscr.refresh()
        curses.noecho()
        curses.cbreak()
        while True:
            try:
                ret = WaitUntil(ioSvc, lambda: cbRcv.count() > 0, period=10)
                if ret:
                    cbTuple = cbRcv.pop()
                    log.append("Updated: {0}".format(cbTuple.path()))
                    tmp = cs.dumpTree()
                    stdscr.clrtobot()
                    stdscr.addstr(0, 0, "Monitor ObexTree at {0}".format(svrInfo))
                    stdscr.addstr(1, 0, "ctrl+c to exit")
                    for idx, t in enumerate(tmp.split("\n")):
                        stdscr.addstr(idx+3, 0, t)
                    stdscr.refresh()
            except:
                break
            # c = stdscr.getch()
            # if c == ord('q'):
            #     break
    except Exception, e:
        print(e)
    curses.echo()
    curses.nocbreak()
    curses.endwin()
if __name__ == "__main__":
    main(sys.argv[1:])

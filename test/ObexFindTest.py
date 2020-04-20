#!/usr/bin/env python

import pdb
from YTest.TestBase import TestBase
from ObjXp import *
import logging
import json


class ObexServerDut(object):
    def __init__(self, progName, sockName, port, ioSvc):
        self._os = CreateObject(objxp.ObexServer, progName, sockName, port, ioSvc)


class ObexClientDut(object):
    def __init__(self, progName, ioSvc):
        self._ioSvc = ioSvc
        self._connCb = CreateObject(objxp.DefaultConnectionCallback)
        self._oc = CreateObject(objxp.ObexClient, progName, ioSvc, self._connCb)

    def getSessionList(self):
        return self._oc.getSessionSummaries().keys()

    def getSession(self, sessionKey):
        return self._oc.getSession(sessionKey)

    def connect(self, sessionKey, arg1, arg2=None, arg3=None):
        if isinstance(arg1, objxp.ObexServer):
            # obexServer = arg1
            self._oc.connect(sessionKey, arg1)
        else:
            # dest = arg1, retryPeriod = arg2, retryLimit = arg3
            self._oc.connect(sessionKey, arg1, arg2, arg3)
        conn = WaitUntil(self._ioSvc, lambda: self._oc.getSession(sessionKey))
        return conn


class ObexFindTest(TestBase):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)
        sockName = "ObexServer"
        self._ioSvc = CreateObject(objxp.BasicIoService)
        self._osd = ObexServerDut("ObexServer", sockName, 0, self._ioSvc)
        self._ocd = ObexClientDut(self.__class__.__name__, self._ioSvc)
        self._rcs = self._ocd.connect("RemoteClientSession", sockName, 3000, 0)
        self._ccs = self._ocd.connect("CohabClientSession", self._osd._os)

    def tearDownCommon(self):
        del self._ccs
        del self._rcs
        del self._ocd
        del self._osd
        del self._ioSvc
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def doObexFindTest(self, cs):
        up = CreateObject(objxp.ObexStringObject, "Up")
        down = CreateObject(objxp.ObexStringObject, "Down")
        cs.putObject("/hw/port/config/et1", up)
        cs.putObject("/hw/port/config/et2", down)
        cs.putObject("/hw/port/config/lag/et3", up)
        cs.putObject("/hw/port/config/lag/et4", up)
        cs.putObject("/hw/port/status/et1", up)
        cs.putObject("/hw/port/status/et2", down)
        cs.putObject("/hw/acl", down)  # out of subscription scope
        cs.putObject("/sw/port/config.old/et1", down)
        cs.putObject("/sw/port/lacp", down)
        cs.putObject("/sw/port.status/.old/.1", up)
        cs.putObject("/sw/port.status/.old/3.", up)
        cs.putObject("/sw/port.status/.new/......", up)
        cs.putObject("/sw/port.status/.new/et.2", up)
        cs.subscribe("/hw/port/*,/sw/*")
        self._ioSvc.run(1000)

        logging.info("  testing invalid path")
        result = cs.find("invalid/path", ".*", True)
        self.assertTrue(result == None)

        logging.info("  testing invalid path(immediate children)")
        result = cs.find("/hw/", ".*", True)
        self.assertTrue(result == None)

        logging.info("  testing another invalid path(all descendants)")
        result = cs.find("/hw/*", ".*", True)
        self.assertTrue(result == None)

        logging.info("  testing recursive mode")
        result = cs.find("/hw/port", ".*", True)
        self.assertTrue(len(result.map) == 6)

        logging.info("  testing non-recursive mode")
        result = cs.find("/hw/port", ".*", False)
        self.assertTrue(result == None)

        logging.info("  testing out-of-subscription")
        result = cs.find("/hw", ".*", True)
        self.assertTrue(len(result.map) == 6)

        logging.info("  testing recursive wildcard")
        result = cs.find("/hw/port", ".*", True)
        self.assertTrue(len(result.map) == 6)
        self.assertTrue(
            set(result.map.keys()) ==
            set(['config/et1', 'config/et2', 'config/lag/et3', 'config/lag/et4',
                 'status/et1', 'status/et2']))

        logging.info("  testing recursive exact match")
        result = cs.find("/hw/port", "et1", True)
        self.assertTrue(set(result.map.keys()) == set(['config/et1', 'status/et1']))

        logging.info("  testing non-recursive wildcard")
        result = cs.find("/hw/port/config", ".*", False)
        self.assertTrue(set(result.map.keys()) == set(['et1', 'et2']))

        logging.info("  testing non-recursive exact")
        result = cs.find("/hw/port/config", "et4", False)
        self.assertTrue(result == None)
        result = cs.find("/hw/port/config/lag", "et4", False)
        self.assertTrue(result.map.keys() == ['et4'])

        logging.info("  testing path name including dot, recursive")
        result = cs.find("/sw", ".*", True)
        self.assertTrue(
                set(result.map.keys()) ==
                set(['port.status/.old/.1',
                     'port.status/.old/3.',
                     'port.status/.new/......',
                     'port.status/.new/et.2',
                     'port/config.old/et1',
                     'port/lacp']))

        logging.info("  testing path name including dot, non-recursive")
        result = cs.find("/sw/port.status/.new", ".*", True)
        self.assertTrue(
            set(result.map.keys()) == set(['et.2', '......']))

    def test_ObexFindRemote(self):
        logging.info("Start ObexFindTest for RemoteClientSession")
        self.doObexFindTest(self._rcs)
        logging.info("End ObexFindTest for RemoteClientSession")

    def test_ObexFindCohab(self):
        logging.info("Start ObexFindTest for CohabClientSession")
        self.doObexFindTest(self._ccs)
        logging.info("End ObexFindTest for CohabClientSession")

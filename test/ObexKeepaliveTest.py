#!/usr/bin/env python

import pdb
from YTest.TestBase import skipIfNotRoot
from YTest.TestBase import TestBase
import objxp
from objxp import DynamicCast_ObexRemoteClientSession
from ObjXp import *
from pyroute2 import IPRoute
import logging
import json
import time
import psutil
import iptc
import os
import time

class ObexKeepaliveTest(TestBase):
    @skipIfNotRoot
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(7)
        self._ip   = '127.0.0.1'
        self._port = 8282
        self._serverAddr = self._ip + ':' + str(self._port)
        self._idle     = 2
        self._interval = 2
        self._probe    = 2
        timeMargin = 1 # for the margin of error
        self._timeout = self._idle + (self._interval * self._probe) + timeMargin

        # iptables -[A|D] OUTPUT -p tcp --sport $port -j DROP
        self._chain = iptc.Chain(iptc.Table(iptc.Table.FILTER), 'OUTPUT')
        self._rule = iptc.Rule()
        self._rule.protocol = 'tcp'
        match = self._rule.create_match('tcp')
        match.sport = str(self._port)
        self._rule.target = iptc.Target(self._rule, 'DROP')

        self._ioSvc = BasicIoService()
        self._os = ObexServer(self.__class__.__name__, '', self._port, self._ioSvc)
        self._oc = ObexClient(self.__class__.__name__, self._ioSvc)

    def tearDownCommon(self):
        del self._oc
        del self._os
        del self._ioSvc
        del self._rule
        del self._chain

        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def getSockStatus(self, port):
        conn = psutil.net_connections()
        for x in conn:
            if x.pid == os.getpid() and len(x.raddr) == 2 and x.raddr[1] == port:
                return x.status

        return "NONE"

    def test_ObexKeepaliveWithIptables(self):
        logging.info("test_ObexKeepaliveWithIptables")
        port = self._port

        session = self._oc.connect('keepalive', self._serverAddr, 3000, 0)
        assert(session)
        WaitUntil(self._ioSvc, lambda: self.getSockStatus(port) == "ESTABLISHED")
        DynamicCast_ObexRemoteClientSession(session).setKeepalive(
            self._idle, self._interval, self._probe)

        try:
            self._chain.insert_rule(self._rule)
            # true: this will disconnect and close socket
            self.assertTrue(WaitUntil(self._ioSvc,
                lambda: self.getSockStatus(port) == "NONE",
                timeout = self._timeout * 1000))
        finally:
            self._chain.delete_rule(self._rule)

    def test_ObexKeepaliveWithoutIptables(self):
        logging.info("test_ObexKeepaliveWithoutIptables")
        port = self._port

        session = self._oc.connect('keepalive', self._serverAddr, 3000, 0)
        assert(session)
        WaitUntil(self._ioSvc, lambda: self.getSockStatus(port) == "ESTABLISHED")
        DynamicCast_ObexRemoteClientSession(session).setKeepalive(
            self._idle, self._interval, self._probe)

        # false: this will stay established state
        self.assertFalse(WaitUntil(self._ioSvc,
            lambda: self.getSockStatus(port) == "NONE",
            timeout = self._timeout * 1000))

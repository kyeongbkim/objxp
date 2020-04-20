#!/usr/bin/env python

import pdb
from YTest.TestBase import TestBase
from ObjXp import *
import logging
import json


class ObexCallbackSrcTest(TestBase):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)
        self._ioSvc = BasicIoService()
        self._cCb = CreateObject(objxp.DefaultConnectionCallback)
        self._sCb = CreateObject(objxp.DefaultSubscriptionCallback)
        self._cbRcv = CreateObject(objxp.ObexCallbackReceiver, "cbRcv")

    def tearDownCommon(self):
        if hasattr(self, '_cbRcv'): del self._cbRcv
        if hasattr(self, '_sCb'): del self._sCb
        if hasattr(self, '_cCb'): del self._cCb
        if hasattr(self, '_ioSvc'): del self._ioSvc
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def test_ObexCallbackSrcTest(self):
        serverName = "ObexTestServer"
        clientName = "ObexTestClient"
        sockName = "sock." + serverName

        ioSvc = self._ioSvc.getIoService()

        self._os = CreateObject(objxp.ObexServer, serverName, sockName, 0, ioSvc)
        self._oc = CreateObject(objxp.ObexClient, clientName, ioSvc, self._cCb)

        self._oc.connect("client1", sockName, 3000, 0)
        self._oc.connect("client2", sockName, 3000, 0)
        self._oc1 = WaitUntil(ioSvc, lambda: self._oc.getSession("client1"))
        self._oc2 = WaitUntil(ioSvc, lambda: self._oc.getSession("client2"))
        self.assertTrue(self._oc1 and self._oc2)

        self._oc1.subscribe("/hello", self._sCb)
        self._oc1.registerCallback("/hello", self._cbRcv)
        self._oc2.subscribe("/hello", self._sCb)
        self._oc2.registerCallback("/hello", self._cbRcv)

        helloObj = CreateObject(objxp.ObexStringObject, "Hello")
        self._oc1.putObject("/hello", helloObj)

        self.assertTrue(WaitUntil(ioSvc, lambda: self._cbRcv.count() >= 2))
        cbSrcs = [self._cbRcv.pop().cbSrc(), self._cbRcv.pop().cbSrc()]
        self.assertTrue(set(cbSrcs) == set(["client1", "client2"]))

        del helloObj
        del self._os
        del self._oc
        del self._oc1
        del self._oc2

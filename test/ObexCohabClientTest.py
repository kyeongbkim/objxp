#!/usr/bin/env python

import pdb
from YTest.TestBase import TestBase
import objxp
from ObjXp import *
import logging
import sys
import json


class ObexCohabClientTest(TestBase):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)

        self.ioSvc_ = CreateObject(objxp.BasicIoService)
        self.connCb_ = CreateObject(objxp.DefaultConnectionCallback)
        self.obexCbRecv_ = CreateObject(objxp.ObexCallbackReceiver, "CbRcv")
        self.helloObj_ = CreateObject(objxp.ObexStringObject, "Hello")
        self.worldObj_ = CreateObject(objxp.ObexStringObject, "World")

        self.sessionKey_ = "cohab"

    def tearDownCommon(self):
        del self.worldObj_
        del self.helloObj_
        del self.obexCbRecv_
        del self.connCb_
        del self.ioSvc_
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def test_ObexCohabClientTest(self):
        logging.info("-- Subscribe-RegCb-PutObj --")

        logging.info("   Exact match test")
        self.SubscribeRegcbPutobjTest(
            "/test/d1/hello", "/test/d1/hello",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   Immediate match test")
        self.SubscribeRegcbPutobjTest(
            "/test/d1/", "/test/d1/",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   All descendants match test")
        self.SubscribeRegcbPutobjTest(
            "/test/*", "/test/*",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   Subset test")
        self.SubscribeRegcbPutobjTest(
            "/test/d1/*", "/test/d1/d2/*",
            [("/test/d1/hello", self.helloObj_),
             ("/test/d1/world", self.worldObj_),
             ("/test/d1/d2/hello", self.helloObj_),
             ("/test/d1/d2/world", self.worldObj_)], 2)
        logging.info("   Inverse subset test")
        self.SubscribeRegcbPutobjTest(
            "/test/d1/d2/*", "/test/d1/*",
            [("/test/d1/hello", self.helloObj_),
             ("/test/d1/world", self.worldObj_),
             ("/test/d1/d2/hello", self.helloObj_),
             ("/test/d1/d2/world", self.worldObj_)], 2)

        logging.info("-- RegCb-PutObj-Subscribe --")
        logging.info("   Exact match test")
        self.regcbPutobjSubscribeTest(
            "/test/d1/hello", "/test/d1/hello",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   Immediate match test")
        self.regcbPutobjSubscribeTest(
            "/test/d1/", "/test/d1/",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   All descendants match test")
        self.regcbPutobjSubscribeTest(
            "/test/*", "/test/*",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   Subset test")
        self.regcbPutobjSubscribeTest(
            "/test/d1/*", "/test/d1/d2/*",
            [("/test/d1/hello", self.helloObj_),
             ("/test/d1/world", self.worldObj_),
             ("/test/d1/d2/hello", self.helloObj_),
             ("/test/d1/d2/world", self.worldObj_)], 2)
        logging.info("   Inverse subset test")
        self.regcbPutobjSubscribeTest(
            "/test/d1/d2/*", "/test/d1/*",
            [("/test/d1/hello", self.helloObj_),
             ("/test/d1/world", self.worldObj_),
             ("/test/d1/d2/hello", self.helloObj_),
             ("/test/d1/d2/world", self.worldObj_)], 2)

        logging.info("-- RegCb-Subscribe-PutObj --")

        logging.info("   Exact match test")
        self.regcbSubscribePutobjTest(
            "/test/d1/hello", "/test/d1/hello",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   Immediate match test")
        self.regcbSubscribePutobjTest(
            "/test/d1/", "/test/d1/",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   All descendants match test")
        self.regcbSubscribePutobjTest(
            "/test/*", "/test/*",
            [("/test/d1/hello", self.helloObj_)], 1)
        logging.info("   Subset test")
        self.regcbSubscribePutobjTest(
            "/test/d1/*", "/test/d1/d2/*",
            [("/test/d1/hello", self.helloObj_),
             ("/test/d1/world", self.worldObj_),
             ("/test/d1/d2/hello", self.helloObj_),
             ("/test/d1/d2/world", self.worldObj_)], 2)
        logging.info("   Inverse subset test")
        self.regcbSubscribePutobjTest(
            "/test/d1/d2/*", "/test/d1/*",
            [("/test/d1/hello", self.helloObj_),
             ("/test/d1/world", self.worldObj_),
             ("/test/d1/d2/hello", self.helloObj_),
             ("/test/d1/d2/world", self.worldObj_)], 2)

    def regcbPutobjSubscribeTest(self,
                                 subscribePath, cbPath, putObjList, expectedCallbackCount):

        os = CreateObject(objxp.ObexServer, "CohabServer", "", 0, self.ioSvc_)
        oc = CreateObject(objxp.ObexClient, "CohabClient", self.ioSvc_, self.connCb_)

        oc.connect(self.sessionKey_, os)
        obexTree = os.getObexTree()
        cohabSession = oc.getSession(self.sessionKey_)

        # Register callback
        cohabSession.registerCallback(cbPath, self.obexCbRecv_)

        # Put objects
        for t in putObjList:
            obexTree.putObject(t[0], t[1])

        result = WaitUntil(self.ioSvc_,
                           lambda: self.obexCbRecv_.count() != 0,
                           timeout=1000)
        assert result == None

        # Subscribe
        sCb = CreateObject(objxp.DefaultSubscriptionCallback)
        cohabSession.subscribe(subscribePath, sCb)
        result = WaitUntil(self.ioSvc_,
                           lambda: self.obexCbRecv_.count(),
                           timeout=1000)
        assert result == expectedCallbackCount, \
            "expected = %d, result = %d" % (expectedCallbackCount, result)
        self.obexCbRecv_.clear()

    def regcbSubscribePutobjTest(self,
                                 subscribePath, cbPath, putObjList, expectedCallbackCount):

        os = CreateObject(objxp.ObexServer, "CohabServer", "", 0, self.ioSvc_)
        oc = CreateObject(objxp.ObexClient, "CohabClient", self.ioSvc_, self.connCb_)

        oc.connect(self.sessionKey_, os)
        obexTree = os.getObexTree()
        cohabSession = oc.getSession(self.sessionKey_)

        # Register callback
        cohabSession.registerCallback(cbPath, self.obexCbRecv_)
        # Subscribe
        sCb = CreateObject(objxp.DefaultSubscriptionCallback)
        cohabSession.subscribe(subscribePath, sCb)
        # Put objects
        for t in putObjList:
            obexTree.putObject(t[0], t[1])

        result = WaitUntil(self.ioSvc_,
                           lambda: self.obexCbRecv_.count(),
                           timeout=1000)
        assert result == expectedCallbackCount, \
            "expected = %d, result = %d" % (expectedCallbackCount, result)
        self.obexCbRecv_.clear()

    def SubscribeRegcbPutobjTest(self,
                                 subscribePath, cbPath, putObjList, expectedCallbackCount):

        os = CreateObject(objxp.ObexServer, "CohabServer", "", 0, self.ioSvc_)
        oc = CreateObject(objxp.ObexClient, "CohabClient", self.ioSvc_, self.connCb_)

        oc.connect(self.sessionKey_, os)
        obexTree = os.getObexTree()
        cohabSession = oc.getSession(self.sessionKey_)

        # Subscribe
        sCb = CreateObject(objxp.DefaultSubscriptionCallback)
        cohabSession.subscribe(subscribePath, sCb)
        # Register callback
        cohabSession.registerCallback(cbPath, self.obexCbRecv_)
        # Put objects
        for t in putObjList:
            obexTree.putObject(t[0], t[1])

        result = WaitUntil(self.ioSvc_,
                           lambda: self.obexCbRecv_.count(),
                           timeout=1000)
        assert result == expectedCallbackCount, \
            "expected = %d, result = %d" % (expectedCallbackCount, result)
        self.obexCbRecv_.clear()

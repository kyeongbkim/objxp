#!/usr/bin/env python

import pdb
import objxp
from objxp import DynamicCast_ObexStringObject
from ObjXp import SetTraceLevel, CreateObject
from ObjXp import EnableObjStats, GetObjStats
from ObjXp import WaitUntil
from YTest.TestBase import TestBase
import logging
import json

class ObexTestCommon(object):
    def _simpleObexTest(self, cs, scb, ocb, sbPath, cbPath,
            putObjs, sbObjs, cbObjs):
        # cs : client session
        # scb : subscription callback
        # ocb : obex callback
        # sbPath : subscription path
        # cbPath : callback path
        # putObjs : [ (path, obj), ... ] - objects to be injected
        # sbObjs  : [ (path, obj), ... ] - subscribed objects
        # cbObjs  : [ (path, obj), ... ] - expected callback objects

        if sbPath and hasattr(cs, 'subscribe'):
            logging.info("  subscribe {}".format(sbPath))
            cs.subscribe(sbPath, scb)
            self.assertTrue(WaitUntil(self._iosvc, lambda: scb.count() == 1))

        logging.info("  register callback path {}".format(cbPath))
        cs.registerCallback(cbPath, ocb)

        for tpl in putObjs:
            path = tpl[0]
            obj  = tpl[1]
            logging.info("  put object {}".format(path))
            cs.putObject(path, obj)

        for tpl in sbObjs:
            path = tpl[0]
            obj  = tpl[1]

            logging.info("  get object {}".format(path))
            sbObj = WaitUntil(self._iosvc,
                    lambda: cs.getObject(path), timeout=30000)
            self.assertTrue(sbObj)
            self.assertTrue(sbObj.equals(obj))
            logging.info("  object {} match success".format(path))

        logging.info("  check callback count")
        self.assertTrue(WaitUntil(self._iosvc,
            lambda: ocb.count() == len(cbObjs)))

        for tpl in cbObjs:
            cbTuple = ocb.pop()
            self.assertTrue(cbTuple)
            path = tpl[0]
            obj  = tpl[1]
            self.assertTrue(cbTuple.path() == path)
            self.assertTrue(
                    DynamicCast_ObexStringObject(cbTuple.obj()).equals(obj))

    def test_BasicObjectTest(self):
        obj1 = CreateObject(objxp.ObexStringObject, "Hello")
        obj2 = CreateObject(objxp.ObexStringObject, "World")
        path1 = "/hardware/portInfo/et1"
        path2 = "/hardware/portInfo/et2"
        sbPath = "/hardware/*"
        cbPath = "/hardware/portInfo/"

        putObjs = [ (path1, obj1), (path2, obj2) ]
        sbObjs  = [ (path1, obj1), (path2, obj2) ]
        cbObjs  = [ (path1, obj1), (path2, obj2) ]
        self._simpleObexTest(self._clientSession, self._scb, self._ocb,
                sbPath, cbPath, putObjs, sbObjs, cbObjs)
        self._clientSession.printTree()
        self._clientSession.saveTree("test-obex-tree.txt")

    def test_BigObjectTest(self):
        obj1 = CreateObject(objxp.ObexStringObject, "0123456789" * 100000)
        obj2 = CreateObject(objxp.ObexStringObject, "abcdefghij" * 100000)
        path1 = "/hardware/portStatus/et1"
        path2 = "/hardware/portInfo/et2"
        sbPath = "/hardware/*"
        cbPath = "/hardware/portInfo/"

        putObjs = [ (path1, obj1), (path2, obj2) ]
        sbObjs  = [ (path1, obj1), (path2, obj2) ]
        cbObjs  = [ (path2, obj2) ]
        self._simpleObexTest(self._clientSession, self._scb, self._ocb,
                sbPath, cbPath, putObjs, sbObjs, cbObjs)

    def test_dotPathTest(self):
        obj1 = CreateObject(objxp.ObexStringObject, "Hello")
        obj2 = CreateObject(objxp.ObexStringObject, "World")
        path1 = "/../hard.ware/port.Info/et.1"
        path2 = "/.././.../hardware/portInfo/et2"
        sbPath = "/../*"
        cbPath = path1

        putObjs = [ (path1, obj1), (path2, obj2) ]
        sbObjs  = [ (path1, obj1), (path2, obj2) ]
        cbObjs  = [ (path1, obj1) ]
        self._simpleObexTest(self._clientSession, self._scb, self._ocb,
                sbPath, cbPath, putObjs, sbObjs, cbObjs)


class ObexTreeTest(TestBase, ObexTestCommon):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)
        self._iosvc = CreateObject(objxp.BasicIoService)
        self._scb = CreateObject(objxp.DefaultSubscriptionCallback);
        self._ocb = CreateObject(objxp.ObexCallbackReceiver, "ocb")
        self._clientSession = CreateObject(objxp.ObexTree)

    def tearDownCommon(self):
        del self._clientSession
        del self._ocb
        del self._scb
        del self._iosvc
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))


class ObexCohabClientTest(TestBase, ObexTestCommon):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)
        self._iosvc = CreateObject(objxp.BasicIoService)
        self._scb = CreateObject(objxp.DefaultSubscriptionCallback);
        self._ccb = CreateObject(objxp.DefaultConnectionCallback)
        self._ocb = CreateObject(objxp.ObexCallbackReceiver, "ocb")
        self._os = CreateObject(objxp.ObexServer, "os", "", 0, self._iosvc)
        self._oc = CreateObject(objxp.ObexClient, "oc", self._iosvc, self._ccb)
        self._sessionKey = "cohab"
        self._oc.connect(self._sessionKey, self._os)
        self._clientSession = self._oc.getSession(self._sessionKey)

    def tearDownCommon(self):
        self._oc.disconnect(self._sessionKey)
        del self._clientSession
        del self._oc
        del self._os
        del self._ocb
        del self._ccb
        del self._scb
        del self._iosvc
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

class ObexRemoteClientTest(TestBase, ObexTestCommon):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)
        self._iosvc = CreateObject(objxp.BasicIoService)
        self._scb = CreateObject(objxp.DefaultSubscriptionCallback);
        self._ccb = CreateObject(objxp.DefaultConnectionCallback)
        self._ocb = CreateObject(objxp.ObexCallbackReceiver, "ocb")
        self._os = CreateObject(objxp.ObexServer, "os", "os", 0, self._iosvc)
        self._oc = CreateObject(objxp.ObexClient, "oc", self._iosvc, self._ccb)
        self._sessionKey = "noncohab"
        self._oc.connect(self._sessionKey, "os", 3000, 0)
        self._clientSession = WaitUntil(self._iosvc,
                lambda: self._oc.getSession(self._sessionKey))
        self.assertTrue(self._clientSession)

    def tearDownCommon(self):
        self._oc.disconnect(self._sessionKey)
        del self._clientSession
        del self._oc
        del self._os
        del self._ocb
        del self._ccb
        del self._scb
        del self._iosvc
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

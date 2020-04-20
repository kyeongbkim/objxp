#!/usr/bin/env python

import pdb
from YTest.TestBase import TestBase
import objxp
from objxp import DynamicCast_ObexJsonObject
from ObjXp import *
import logging
import sys
import json

sessionId = "ObexInfoProviderTest"
YpProcSession__SID__Cmd = "/proc/session/%s/cmd"
YpProcSession__SID__Rsp = "/proc/session/%s/rsp"


class ObexInfoProviderCmd:
    jsonObj_ = None

    def __init__(self, cmdId, cmdStr):
        cmd = '{"id":"%s", "cmd":"%s"}' % (cmdId, cmdStr)
        self.jsonObj_ = CreateObject(objxp.ObexJsonObject, cmd)

    def getObj(self):
        return self.jsonObj_


class ObexInfoProviderTest(TestBase):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)
        self.seqNo_ = 0
        self.cmdPath_ = YpProcSession__SID__Cmd % sessionId
        self.rspPath_ = YpProcSession__SID__Rsp % sessionId
        self.obexTree_ = CreateObject(objxp.ObexTree)
        self.obexInfoProvider_ = CreateObject(objxp.ObexInfoProvider, self.obexTree_)
        self.obexCbRecv_ = CreateObject(objxp.ObexCallbackReceiver, sessionId)
        self.obexTree_.registerCallback(self.rspPath_, self.obexCbRecv_)

    def tearDownCommon(self):
        del self.obexCbRecv_
        del self.obexInfoProvider_
        del self.obexTree_
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def _sendCmd(self, cmdStr):
        self.seqNo_ += 1
        cmdObj = ObexInfoProviderCmd(str(self.seqNo_), cmdStr)
        self.obexTree_.putObject(self.cmdPath_, cmdObj.getObj())

    def _verifyCmd(self, rsp, err=None):
        assert self.obexCbRecv_.count() == 1
        cbTuple = self.obexCbRecv_.pop()
        assert cbTuple.path() == self.rspPath_
        assert cbTuple.obj()
        jObj = json.loads(DynamicCast_ObexJsonObject(cbTuple.obj()).getString())
        assert jObj['id'] == str(self.seqNo_)
        if rsp != None:
            assert set(rsp).issubset(set(jObj['rsp']))
        else:
            assert 'rsp' not in jObj
        if err == None:
            assert 'err' not in jObj
        else:
            assert jObj['err'].startswith(err)

    def test_ObexInfoProviderTest(self):
        logging.info("Start " + self.__class__.__name__)

        logging.info("  testing root entry")
        self._sendCmd("ls /")
        self._verifyCmd(['proc'])

        logging.info("  testing non-leaf entry")
        self._sendCmd("ls /proc")
        self._verifyCmd(['session'])

        self._sendCmd("ls /proc/session")
        self._verifyCmd([sessionId])

        logging.info("  testing leaf entry")
        self._sendCmd("ls " + self.cmdPath_)
        self._verifyCmd([])

        logging.info("  testing non-existing entry")
        self._sendCmd("ls /non/existing/node")
        self._verifyCmd(None, err="2 - No such file")

        logging.info("  testing multiple entries")
        hello = CreateObject(objxp.ObexStringObject, "hello")
        world = CreateObject(objxp.ObexStringObject, "world")
        self.obexTree_.putObject("/some/other/place/hello", hello)
        self.obexTree_.putObject("/some/other/place/world", world)
        self._sendCmd("ls /some/other/place")
        self._verifyCmd(['hello', 'world'])

        logging.info("End " + self.__class__.__name__ + ' : PASS')


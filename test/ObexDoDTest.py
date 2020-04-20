#!/usr/bin/env python

import pdb
from YTest.TestBase import TestBase
import objxp
from ObjXp import *
import logging
import json


class ObexDoDTest(TestBase):
    def _connectTestSession(self, sessionKey, sockName):
        ocsTest = self._oc.connect(sessionKey, sockName, 1000, 0)
        self.assertTrue(ocsTest)
        ocsTest = objxp.DynamicCast_ObexRemoteClientSession(ocsTest)
        self.assertTrue(ocsTest)
        return ocsTest

    def _disconnectTestSession(self, sessionKey):
        self._oc.disconnect(sessionKey)

    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)

        self._ioSvc = BasicIoService()

        self._osName = self.__class__.__name__ + 'Server'
        self._sockName = self._osName
        self._ocName = self.__class__.__name__ + 'Client'
        self._ocskTest = 'ocskTest'
        self._ocskProbe = 'ocskProbe'
        self._path = '/service/dodTest'

        logging.info("  Creating ObexServer ...")
        self._os = ObexServer(self._osName, self._sockName, 0, self._ioSvc)

        logging.info("  Creating ObexClient ...")
        self._oc = ObexClient(self._ocName, self._ioSvc)

        logging.info("  Creating ObexProbe ...")
        self._ocsProbe = self._oc.connect(self._ocskProbe, self._sockName, 1000, 0)
        self.assertTrue(self._ocsProbe)
        self._ocsProbe.subscribe(self._path + '/')

    def tearDownCommon(self):
        del self._ocsProbe
        del self._oc
        del self._os
        del self._ioSvc

        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def test_ObexDoDTest_1(self):
        logging.info("Begin test_ObexDoDTest")

        ioSvc = self._ioSvc.getIoService()

        helloPath = self._path + '/hello'
        helloObj = CreateObject(objxp.ObexStringObject, 'Hello')
        worldPath = self._path + '/world'
        worldObj = CreateObject(objxp.ObexStringObject, 'World')

        logging.info("  Creating ObexClient session ...")
        ocsTest = self._connectTestSession(self._ocskTest, self._sockName)
        self.assertTrue(ocsTest)

        logging.info("  put helloObj ...")
        ocsTest.putObject(helloPath, helloObj)
        logging.info("  put worldObj with delOnDisconnect ...")
        ocsTest.putObject(worldPath, worldObj, True)
        ioSvc.run(1000)

        self.assertTrue(self._ocsProbe.getObject(helloPath))
        logging.info("  helloObj created successfully ...")
        self.assertTrue(self._ocsProbe.getObject(worldPath))
        logging.info("  worldObj created successfully ...")

        logging.info("  disconnecting the session ...")
        self._disconnectTestSession(self._ocskTest)
        ioSvc.run(1000)

        self.assertTrue(self._ocsProbe.getObject(helloPath))
        logging.info("  helloObj should still exist ...")
        self.assertFalse(self._ocsProbe.getObject(worldPath))
        logging.info("  worldObj should be gone ...")

        logging.info("End test_ObexDoDTest")

    def test_ObexDoDTest_2(self):
        logging.info("Begin test_ObexDoDTest")

        ioSvc = self._ioSvc.getIoService()

        helloPath = self._path + '/hello'
        helloObj = CreateObject(objxp.ObexStringObject, 'Hello')

        logging.info("  Creating ObexClient session ...")
        ocsTest = self._connectTestSession(self._ocskTest, self._sockName)
        self.assertTrue(ocsTest)

        logging.info("  put helloObj with delOnDisconnect ...")
        ocsTest.putObject(helloPath, helloObj, True)
        ioSvc.run(1000)
        self.assertTrue(self._ocsProbe.getObject(helloPath))
        logging.info("  helloObj created successfully ...")

        logging.info("  overwrite helloObj without delOnDisconnect ...")
        helloObj = CreateObject(objxp.ObexStringObject, 'Hi')
        ocsTest.putObject(helloPath, helloObj)
        ioSvc.run(1000)
        self.assertTrue(self._ocsProbe.getObject(helloPath))
        logging.info("  helloObj created successfully ...")

        self._disconnectTestSession(self._ocskTest)
        ioSvc.run(1000)

        self.assertTrue(self._ocsProbe.getObject(helloPath))
        logging.info("  helloObj should still exist ...")

        logging.info("End test_ObexDoDTest")

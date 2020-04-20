#!/usr/bin/env python

import pdb
from YTest.TestBase import skipIfNotRoot, skip_common
from YTest.TestBase import TestBase
import objxp
from ObjXp import *
from pyroute2 import IPRoute
import RtgUtil
import logging
import json
import time
import os


class ObexClientTest(TestBase):
    @skipIfNotRoot
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(9)

        self._ioSvc = BasicIoService()
        self._oc = ObexClient(self.__class__.__name__, self._ioSvc)
        self._tm = RtgUtil.TapManager()

        self._bpIfName = 'ocTest'
        self._bpIfAddr = '192.168.100.1'
        self._bpIfMask = 24
        self._bpRemotePeer = '192.168.100.2:9437'
        self._bpLocalPeer = 'NonExistingSocket'

        self._tm.attachInterface(self._bpIfName)
        self.assertTrue(WaitUntil(self._ioSvc,
                                  lambda: len(self._tm.getIntfList()) == 1))

        self._ip = IPRoute()
        self._bpIfIdx = self._ip.link_lookup(ifname=self._bpIfName)[0]
        self._ip.addr('add', index=self._bpIfIdx,
                      address=self._bpIfAddr, mask=self._bpIfMask)
        self._ip.link('set', index=self._bpIfIdx, state='up')

    def tearDownCommon(self):
        self._tm.detachInterface(self._bpIfName)
        del self._ip
        del self._tm
        del self._oc
        del self._ioSvc

        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def test_ObexRemoteServerUnreachable(self):
        oc = self._oc.getObexClient()
        self.assertTrue(type(oc) == objxp.ObexClient)

        begin = time.time()
        oc.connect('UnreachableRemoteSession', self._bpRemotePeer, 0, 0)
        self._ioSvc.run(1)
        elapsedTime = time.time() - begin
        self.assertTrue(elapsedTime < 0.1)  # should be non-blocking

    def test_ObexLocalServerUnreachable(self):
        oc = self._oc.getObexClient()
        self.assertTrue(type(oc) == objxp.ObexClient)

        begin = time.time()
        oc.connect('UnreachableLocalSession', self._bpLocalPeer, 0, 0)
        self._ioSvc.run(1)
        elapsedTime = time.time() - begin
        self.assertTrue(elapsedTime < 0.1)  # should be non-blocking

    @skip_common
    def setUp_ClientNotResponding(self):
        EnableObjStats()
        SetTraceLevel(8)

        self._ioSvc = BasicIoService()
        self._ioSvcStalled = BasicIoService()

        self._osName = self.__class__.__name__ + 'Server'
        self._sockName = self._osName
        self._sockPort = 6464
        self._sockAddr = '127.0.0.1:' + str(self._sockPort)
        self._ocName = self.__class__.__name__ + 'Client'
        self._ocNameStalled = self.__class__.__name__ + 'ClientStalled'

        self._pathPrefix = '/service/nrTest'
        self._subscribePath = self._pathPrefix + '/*'

        logging.info('  Creating ObexServer ...')
        self._os = ObexServer(self._osName, self._sockName,
                              self._sockPort, self._ioSvc)
        self._oc1 = ObexClient(self._ocName, self._ioSvc)
        self._oc2 = ObexClient(self._ocNameStalled, self._ioSvcStalled)

        self._ioSvc.run(500)
        self._ioSvcStalled.run(500)

    @skip_common
    def tearDown_ClientNotResponding(self):
        del self._oc1, self._oc2, self._os
        del self._ioSvc, self._ioSvcStalled

        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def test_ClientNotResponding(self):
        logging.info('Begin test_ClientNotResponding')

        ocsUnix1 = self._oc1.connect('usock', self._sockName, 0, 0)
        self.assertTrue(ocsUnix1)
        ocsUnix1.subscribe(self._subscribePath)
        logging.info('  usock client 1 connected')

        ocsTcp1  = self._oc1.connect('tcp', self._sockAddr, 0, 0)
        self.assertTrue(ocsTcp1)
        ocsTcp1.subscribe(self._subscribePath)
        logging.info('  tcp client 1 connected')

        ocsUnix2 = self._oc2.connect('usock', self._sockName, 0, 0)
        self.assertTrue(ocsUnix2)
        ocsUnix2.subscribe(self._subscribePath)
        logging.info('  usock client 2 connected')

        ocsTcp2  = self._oc2.connect('tcp', self._sockAddr, 0, 0)
        self.assertTrue(ocsTcp2)
        ocsTcp2.subscribe(self._subscribePath)
        logging.info('  tcp client 2 connected')

        self._ioSvc.run(500)
        self._ioSvcStalled.run(500)

        logging.info('  create an object and put to the obex server')
        obj = CreateObject(objxp.ObexStringObject, '.' * 8192)
        objPath = self._pathPrefix + '/hello'
        ocsUnix1.putObject(objPath, obj)

        self.assertTrue(
                WaitUntil(self._ioSvc,
                    lambda: ocsUnix1.getObject(objPath)))
        self.assertTrue(
                WaitUntil(self._ioSvc,
                    lambda: ocsTcp1.getObject(objPath)))
        self.assertTrue(
                WaitUntil(self._ioSvcStalled,
                    lambda: ocsUnix2.getObject(objPath)))
        self.assertTrue(
                WaitUntil(self._ioSvcStalled,
                    lambda: ocsTcp2.getObject(objPath)))
        logging.info('  all clients should receive the object')

        logging.info('  delete the object')
        ocsUnix1.delObject(objPath)
        self.assertTrue(
                WaitUntil(self._ioSvc,
                    lambda: not ocsUnix1.getObject(objPath)))
        self.assertTrue(
                WaitUntil(self._ioSvc,
                    lambda: not ocsTcp1.getObject(objPath)))
        self.assertTrue(
                WaitUntil(self._ioSvcStalled,
                    lambda: not ocsUnix2.getObject(objPath)))
        self.assertTrue(
                WaitUntil(self._ioSvcStalled,
                    lambda: not ocsTcp2.getObject(objPath)))
        logging.info('  all clients should have the object removed')

        sessionCnt = len(self._os.getSessionList())
        loopCnt = 0
        while True:
            ocsUnix1.putObject(objPath, obj)
            ocsUnix1.delObject(objPath)
            self._ioSvc.run(100)
            loopCnt += 1

            if sessionCnt != len(self._os.getSessionList()):
                sessionCnt = len(self._os.getSessionList())
                # Maybe unix socket session gets disconnected first
                # because the buffer size of unix socket is usually
                # less than the tcp socket buffer
                logging.info('  connection lost, loopCnt = ' + str(loopCnt))
                if sessionCnt < 3:
                    break;

            if loopCnt % 100 == 0:
                logging.info('  loopCnt = ' + str(loopCnt))
                self.assertTrue(loopCnt <= 1000)

        logging.info('  stalled sessions should be disconnected')
        self.assertTrue(WaitUntil(self._ioSvcStalled,
                                  lambda: len(self._oc2.getSessionList()) == 0))
        logging.info('  stalled sessions removed successfully')

        logging.info('  normal clients should not be impacted')
        objPath = self._pathPrefix + '/world'
        ocsUnix1.putObject(objPath, obj)

        self.assertTrue(
                WaitUntil(self._ioSvc,
                    lambda: ocsUnix1.getObject(objPath)))
        self.assertTrue(
                WaitUntil(self._ioSvc,
                    lambda: ocsTcp1.getObject(objPath)))
        logging.info('  normal clients are working good')

        logging.info('End test_ClientNotResponding')

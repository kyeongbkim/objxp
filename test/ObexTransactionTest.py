#!/usr/bin/env python

import pdb
import objxp
from objxp import DynamicCast_ObexStringObject
from ObjXp import SetTraceLevel, CreateObject
from ObjXp import EnableObjStats, GetObjStats
from ObjXp import ObexServer, ObexClient
from ObjXp import WaitUntil
from YTest.TestBase import TestBase
import logging
import json

class ObexServerSimulator(object):
    def __init__(self, name, port, ioSvc):
        self._ioSvc = ioSvc
        self._osName = self.__class__.__name__ + '-' + name
        self._sockName = self._osName
        self._sockPort = port
        self._sockAddr = '127.0.0.1:' + str(self._sockPort)
        self._os = ObexServer(self._osName, self._sockName,
                              self._sockPort, self._ioSvc)

    def getUnixSockAddr(self):
        return self._sockName

    def getTcpSockAddr(self):
        return '127.0.0.1:' + str(self._sockPort)

    def getInjector(self, injName, sockType):
        if not hasattr(self, '_oc') or not self._oc:
            self._oc = CreateObject(objxp.ObexClient, injName, self._ioSvc, None)
        if sockType == 'unix':
            self._oc.connect(injName, self.getUnixSockAddr(), 0, 0)
        elif sockType == 'tcp':
            self._oc.connect(injName, self.getTcpSockAddr(), 0, 0)
        else:
            self.assertTrue(False)

        cs = WaitUntil(self._ioSvc, lambda: self._oc.getSession(injName))
        return objxp.DynamicCast_ObexRemoteClientSession(cs)

    def delInjector(self, injName):
        if hasattr(self, '_oc'):
            self._oc.disconnect(injName)

class ObexClientSimulator(object):
    def __init__(self, name, sockAddr, subscribePath, ioSvc):
        self._ioSvc = ioSvc
        self._ccb = CreateObject(objxp.DefaultConnectionCallback)
        self._scb = CreateObject(objxp.DefaultSubscriptionCallback);
        self._ocb = CreateObject(objxp.ObexCallbackReceiver, name + 'ocb')
        self._oc  = CreateObject(objxp.ObexClient, name, ioSvc, self._ccb)

        self._oc.connect(name, sockAddr, 3000, 0)
        cs = WaitUntil(self._ioSvc, lambda: self._oc.getSession(name))
        self._cs = objxp.DynamicCast_ObexRemoteClientSession(cs)
        self._cs.subscribe(subscribePath, self._scb)
        self._cs.registerCallback(subscribePath, self._ocb)

    def callbackObjCount(self):
        return self._ocb.count()

    def popCallbackObj(self):
        if self.callbackObjCount() > 0:
            cbObj = self._ocb.pop()
            return cbObj.path(), cbObj.obj()
        else:
            return "", None

    def clearCallbackObj(self):
        self._ocb.clear()

    def getObject(self, path):
        return self._cs.getObject(path)

class ObexTransactionTest(TestBase):
    def setUpCommon(self):
        EnableObjStats()
        SetTraceLevel(8)

        self._ioSvc = CreateObject(objxp.BasicIoService)

        self._objDir = '/xactionTest'
        self._objDirA = '/xactionTest/a'
        self._objDirB = '/xactionTest/b'
        self._objDirC = '/xactionTest/c'

        self._sSim = ObexServerSimulator('xs', 6464, self._ioSvc)
        self._cSim  = ObexClientSimulator('xc1', self._sSim.getUnixSockAddr(),
                                          self._objDir + '/*', self._ioSvc)
        self._cSimA = ObexClientSimulator('xc2', self._sSim.getTcpSockAddr(),
                                          self._objDirA + '/*', self._ioSvc)
        self._cSimB = ObexClientSimulator('xc3', self._sSim.getTcpSockAddr(), 
                                          self._objDirB + '/*', self._ioSvc)


    def tearDownCommon(self):
        del self._cSimB
        del self._cSimA
        del self._cSim
        del self._sSim
        del self._ioSvc
        logging.info("ObjStats: " + GetObjStats())
        self.assertFalse(bool(json.loads(GetObjStats())))

    def test_BasicTransactionTest(self):
        obj1 = CreateObject(objxp.ObexStringObject, "Hello")
        inj1 = self._sSim.getInjector("inj1", 'unix')

        self.assertTrue(self._cSim.callbackObjCount() == 0)
        self.assertTrue(self._cSimA.callbackObjCount() == 0)
        self.assertTrue(self._cSimB.callbackObjCount() == 0)

        objPathA = self._objDirA + '/1'
        objPathB = self._objDirB + '/1'

        logging.info('  begin transaction')
        inj1.beginTransaction()

        logging.info('  put an object')
        inj1.putObject(objPathA, obj1)
        self._ioSvc.run(1000)
        self.assertTrue(self._cSim.callbackObjCount() == 0)
        self.assertTrue(self._cSimA.callbackObjCount() == 0)
        self.assertTrue(self._cSimB.callbackObjCount() == 0)
        logging.info('  nothing actually put yet')

        logging.info('  put another object')
        inj1.putObject(objPathB, obj1)
        self._ioSvc.run(1000)
        self.assertTrue(self._cSim.callbackObjCount() == 0)
        self.assertTrue(self._cSimA.callbackObjCount() == 0)
        self.assertTrue(self._cSimB.callbackObjCount() == 0)
        logging.info('  nothing actually put yet')

        logging.info('  end transaction')
        inj1.endTransaction()
        self._ioSvc.run(1000)
        self.assertTrue(self._cSim.callbackObjCount() == 2)
        self.assertTrue(self._cSimA.callbackObjCount() == 1)
        self.assertTrue(self._cSimB.callbackObjCount() == 1)
        logging.info('  received callback objects')

        self.assertTrue(self._cSim.getObject(objPathA) != None)
        self.assertTrue(self._cSim.getObject(objPathB) != None)
        self.assertTrue(self._cSimA.getObject(objPathA) != None)
        self.assertTrue(self._cSimA.getObject(objPathB) == None)
        self.assertTrue(self._cSimB.getObject(objPathA) == None)
        self.assertTrue(self._cSimB.getObject(objPathB) != None)
        logging.info('  all objects were propagated')

    def _transactionOverlapTest(self, obj1, obj2, timeout=10000):
        inj1 = self._sSim.getInjector("inj1", 'unix')
        inj2 = self._sSim.getInjector("inj2", 'tcp')

        objPathA = self._objDirA + '/1'
        objPathB = self._objDirB + '/1'

        self.assertTrue(self._cSim.callbackObjCount() == 0)
        self.assertTrue(self._cSimA.callbackObjCount() == 0)
        self.assertTrue(self._cSimB.callbackObjCount() == 0)

        logging.info('  begin transaction_1')
        inj1.beginTransaction()

        logging.info('  transaction_1.putObject(pathA, obj1)')
        inj1.putObject(objPathA, obj1)

        logging.info('  begin transaction_2')
        inj2.beginTransaction()

        logging.info('  transaction_1.putObject(pathB, obj1)')
        inj1.putObject(objPathB, obj1)
        logging.info('  transaction_2.putObject(pathA, obj2)')
        inj2.putObject(objPathA, obj2)
        logging.info('  transaction_2.putObject(pathB, obj2)')
        inj2.putObject(objPathB, obj2)

        self._ioSvc.run(1000)
        self.assertTrue(self._cSim.callbackObjCount() == 0)
        self.assertTrue(self._cSimA.callbackObjCount() == 0)
        self.assertTrue(self._cSimB.callbackObjCount() == 0)
        logging.info('  no object received because nothing committed yet')

        logging.info('  committing transaction_2 first')
        inj2.endTransaction()

        logging.info('  verifying obj2')
        WaitUntil(self._ioSvc, lambda: self._cSim.callbackObjCount() == 2,
                  timeout=timeout)
        p, o = self._cSim.popCallbackObj()
        self.assertTrue(p == objPathA)
        self.assertTrue(o.toString() == obj2.toString())
        p, o = self._cSim.popCallbackObj()
        self.assertTrue(p == objPathB)
        self.assertTrue(o.toString() == obj2.toString())

        logging.info('  verifying cSimA.obj2')
        WaitUntil(self._ioSvc, lambda: self._cSimA.callbackObjCount() == 1,
                  timeout=timeout)
        p, o = self._cSimA.popCallbackObj()
        self.assertTrue(p == objPathA)
        self.assertTrue(o.toString() == obj2.toString())

        logging.info('  verifying cSimB.obj2')
        WaitUntil(self._ioSvc, lambda: self._cSimB.callbackObjCount() == 1,
                  timeout=timeout)
        p, o = self._cSimB.popCallbackObj()
        self.assertTrue(p == objPathB)
        self.assertTrue(o.toString() == obj2.toString())

        logging.info('  committing transaction_1')
        inj1.endTransaction()

        logging.info('  verifying obj1')
        WaitUntil(self._ioSvc, lambda: self._cSim.callbackObjCount() == 2,
                  timeout=timeout)
        p, o = self._cSim.popCallbackObj()
        self.assertTrue(p == objPathA)
        self.assertTrue(o.toString() == obj1.toString())
        p, o = self._cSim.popCallbackObj()
        self.assertTrue(p == objPathB)
        self.assertTrue(o.toString() == obj1.toString())

        logging.info('  verifying cSimA.obj1')
        WaitUntil(self._ioSvc, lambda: self._cSimA.callbackObjCount() == 1,
                  timeout=timeout)
        p, o = self._cSimA.popCallbackObj()
        self.assertTrue(p == objPathA)
        self.assertTrue(o.toString() == obj1.toString())

        logging.info('  verifying cSimB.obj1')
        WaitUntil(self._ioSvc, lambda: self._cSimB.callbackObjCount() == 1,
                  timeout=timeout)
        p, o = self._cSimB.popCallbackObj()
        self.assertTrue(p == objPathB)
        self.assertTrue(o.toString() == obj1.toString())

    def test_TransactionOverlapTest(self):
        obj1 = CreateObject(objxp.ObexStringObject, "Hello")
        obj2 = CreateObject(objxp.ObexStringObject, "World")
        self._transactionOverlapTest(obj1, obj2)

    def test_DeleteObjectTest(self):
        obj1 = CreateObject(objxp.ObexStringObject, "Hello")
        obj2 = CreateObject(objxp.ObexStringObject, "World")
        objPathA = self._objDirA + '/1'
        objPathB = self._objDirB + '/1'
        objPathC = self._objDirC + '/1'

        inj1 = self._sSim.getInjector("inj1", 'unix')
        inj1.putObject(objPathA, obj1)
        inj1.putObject(objPathB, obj1)
        inj1.putObject(objPathC, obj1)
        self._ioSvc.run(1000)
        self.assertTrue(self._cSim.callbackObjCount() == 3)
        self.assertTrue(self._cSimA.callbackObjCount() == 1)
        self.assertTrue(self._cSimB.callbackObjCount() == 1)
        self._cSim.clearCallbackObj()
        self._cSimA.clearCallbackObj()
        self._cSimB.clearCallbackObj()
        self.assertTrue(self._cSim.getObject(objPathA) != None)
        self.assertTrue(self._cSim.getObject(objPathB) != None)
        self.assertTrue(self._cSim.getObject(objPathC) != None)

        logging.info('  begin transaction')
        inj1.beginTransaction()
        inj1.putObject(objPathA, obj2, True)
        inj1.delObject(objPathB)
        logging.info('  end transaction')
        inj1.endTransaction()
        self._ioSvc.run(1000)

        logging.info('  verifying delObject')
        self.assertTrue(self._cSim.getObject(objPathA).toString() == \
                        obj2.toString())
        self.assertTrue(self._cSim.getObject(objPathB) == None)
        self.assertTrue(self._cSim.getObject(objPathC).toString() == \
                        obj1.toString())

        logging.info('  disconnect injector')
        self._sSim.delInjector('inj1')
        inj1 = None
        self._ioSvc.run(1000)

        logging.info('  verifying delOnDisconnect')
        self.assertTrue(self._cSim.getObject(objPathA) == None)
        self.assertTrue(self._cSim.getObject(objPathB) == None)
        self.assertTrue(self._cSim.getObject(objPathC).toString() == \
                        obj1.toString())

    def test_BigObjectTest(self):
        obj1 = CreateObject(objxp.ObexStringObject, "Hello" * 1024 * 1024)
        obj2 = CreateObject(objxp.ObexStringObject, "0123456789" * 1024 * 16)
        self._transactionOverlapTest(obj1, obj2, timeout=60000)


import pdb
import time
import signal
import objxp
import gc

def SetTraceLevel(level):
    objxp.SetTraceLevel(level)


def EnableObjStats():
    objxp.EnableObjStats(True)


def DisableObjStats():
    objxp.EnableObjStats(False)


def GetObjStats(verbose=False):
    gc.collect()
    return objxp.GetObjStats(verbose)


def CreateObject(typeName, *args):
    obj = typeName()
    obj.setThisPtr(obj)
    obj.init(*args)
    return obj


def WaitUntil(ioSvc, func, timeout=10000, period=100, *args, **kwargs):
    msec = 1000.0
    expiry = time.time() + (timeout / msec)
    while True:
        remaining = int((expiry - time.time()) * msec)
        if remaining <= 0: break
        result = func(*args, **kwargs)
        if result: return result
        ioSvc.run(min(period, remaining))
        period *= 2

    # check one more time before return
    result = func(*args, **kwargs)
    if result: return result
    return None


def _BasicIoServiceSigHandler(signum, frame):
    pass


class BasicIoService(object):
    def __init__(self):
        self._ioSvc = CreateObject(objxp.BasicIoService)

    def getIoService(self):
        return self._ioSvc

    def run(self, duration=0):
        origHandler = signal.getsignal(signal.SIGINT)
        if origHandler == signal.default_int_handler:
            signal.signal(signal.SIGINT, _BasicIoServiceSigHandler)

        timeQuantum = 100
        beginTime = time.time() * 1000  # in milliseconds
        elapsedTime = 0
        processingCount = 0

        while True:
            timeout = timeQuantum
            if duration > 0:
                remainingTime = duration - elapsedTime
                if remainingTime < timeQuantum:
                    timeout = int(remainingTime)

            if (self._ioSvc.run(timeout if timeout else 1) != 0):
                # select interrupted
                break

            elapsedTime = time.time() * 1000 - beginTime  # in milliseconds
            if duration > 0 and elapsedTime >= duration:
                break

        if origHandler == signal.default_int_handler:
            signal.signal(signal.SIGINT, origHandler)  # Restore handler


class ObexServer(object):
    def __init__(self, progName, sockName, port, ioSvc, fwdOnly=True):
        self._ioSvc = ioSvc if isinstance(ioSvc, objxp.IoService) \
            else ioSvc.getIoService()
        self._os = CreateObject(objxp.ObexServer, progName, sockName, port,
                                self._ioSvc, fwdOnly)

    def getObexServer(self):
        return self._os

    def getSessionList(self):
        ret = []
        ss = self._os.getSessionSummaries()
        for k in ss.keys():
            ret.append((k, ss[k]))
        return ret


class ObexClient(object):
    def __init__(self, progName, ioSvc, connCb=None):
        self._ioSvc = ioSvc if isinstance(ioSvc, objxp.IoService) \
            else ioSvc.getIoService()
        self._connCb = connCb if connCb \
            else CreateObject(objxp.DefaultConnectionCallback)
        self._oc = CreateObject(objxp.ObexClient, progName,
                                self._ioSvc, self._connCb)

    def getObexClient(self):
        return self._oc

    def getSessionList(self):
        ret = []
        ss = self._oc.getSessionSummaries()
        for k in ss.keys():
            ret.append((k, ss[k]))
        return ret

    def getSession(self, sessionKey, connectedOnly=False):
        if connectedOnly:
            ss = self._oc.getSessionSummaries()
            for k in ss.keys():
                if sessionKey == k and ss[k].startswith('Connected'):
                    return self._oc.getSession(k)
            return None
        else:
            return self._oc.getSession(sessionKey)

    def connect(self, sessionKey, arg1, arg2=None, arg3=None):
        if isinstance(arg1, objxp.ObexServer):
            # obexServer = arg1
            self._oc.connect(sessionKey, arg1)
        elif isinstance(arg1, ObexServer):
            self._oc.connect(sessionKey, arg1.getObexServer())
        else:
            # dest = arg1, retryPeriod = arg2, retryLimit = arg3
            self._oc.connect(sessionKey, arg1, arg2, arg3)
        session = WaitUntil(self._ioSvc,
                            lambda: self.getSession(sessionKey, connectedOnly=True))
        return session

    def disconnect(self, sessionKey):
        self._oc.disconnect(sessionKey)

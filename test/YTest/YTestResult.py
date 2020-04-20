import pdb
import os
import unittest
import tempfile


class YTestResult(unittest.TextTestResult):
    _DEBUG = False

    STDOUT_FILENO = 1
    STDERR_FILENO = 2
    reportUrl = None

    def __init__(self, stream, descriptions, verbosity):
        self._fdOut = os.dup(self.STDOUT_FILENO)
        self._fdErr = os.dup(self.STDERR_FILENO)

        fd, filename = tempfile.mkstemp('.txt')  # Create a temp file
        os.close(fd)  # close it
        self._fdBuf = os.open(filename, os.O_RDWR, 0)  # Reopen in non-buffered mode
        os.unlink(filename)  # Make sure to remove it
        # when all its descriptors are closed.

        super(YTestResult, self).__init__(stream, descriptions, verbosity)

    def _redirectOutput(self):
        if self._DEBUG: return
        os.dup2(self._fdBuf, self.STDOUT_FILENO)
        os.dup2(self._fdBuf, self.STDERR_FILENO)

    def _restoreOutput(self):
        if self._DEBUG: return
        os.dup2(self._fdOut, self.STDOUT_FILENO)
        os.dup2(self._fdErr, self.STDERR_FILENO)

    def _readBuffer(self):
        if self._DEBUG: return ''

        n = os.lseek(self._fdBuf, 0, os.SEEK_CUR)
        assert n == os.lseek(self.STDOUT_FILENO, 0, os.SEEK_CUR)
        assert n == os.lseek(self.STDERR_FILENO, 0, os.SEEK_CUR)

        os.lseek(self._fdBuf, 0, os.SEEK_SET)
        data = os.read(self._fdBuf, n)
        assert len(data) == n

        os.lseek(self._fdBuf, 0, os.SEEK_SET)
        os.ftruncate(self._fdBuf, 0)
        return data

    def _reportResult(self, test, result, **kwargs):
        if self._DEBUG: return

        data = self._readBuffer()

        # FIXME
        f = open('./' + test.getTestName() + '.log', 'w')
        f.write('# FIXME - reporting to ' +
                self.reportUrl + '/' + test.getTestName() + '\n')
        f.write(data)
        f.close()

    def startTest(self, test):
        self._redirectOutput()
        super(YTestResult, self).startTest(test)

    def stopTest(self, test):
        super(YTestResult, self).stopTest(test)
        self._restoreOutput()

    def addError(self, test, err):
        super(YTestResult, self).addError(test, err)
        self._reportResult(test, 'Error', err=err)

    def addFailure(self, test, err):
        super(YTestResult, self).addFailure(test, err)
        self._reportResult(test, 'Failure', err=err)

    def addSuccess(self, test):
        super(YTestResult, self).addSuccess(test)
        self._reportResult(test, 'Success')

    def addSkip(self, test, reason):
        super(YTestResult, self).addSkip(test, reason)
        self._reportResult(test, 'Skip', reason=reason)

    def addExpectedFailure(self, test, err):
        super(YTestResult, self).addExpectedFailure(test, err)
        self._reportResult(test, 'ExpectedFailure', err=err)

    def addUnexpectedSuccess(self, test):
        super(YTestResult, self).addUnexpectedSuccess(test)
        self._reportResult(test, 'UnexpectedSuccess')

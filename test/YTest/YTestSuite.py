import pdb
import unittest


class YTestSuite(unittest.TestSuite):
    def __init__(self, setUpSuite=None, tearDownSuite=None):
        self._params = []
        self.setUpSuite = setUpSuite
        self.tearDownSuite = tearDownSuite
        super(YTestSuite, self).__init__()

    def addTest(self, test, param=None):
        if not isinstance(test, unittest.TestCase):
            assert param == None
        self._params.append(param)
        super(YTestSuite, self).addTest(test)

    # Overriding unittest.TestSuite.run
    # Mostly the same except setting up test._param
    def run(self, result, debug=False):
        topLevel = False
        if getattr(result, '_testRunEntered', False) is False:
            result._testRunEntered = topLevel = True

        if self.setUpSuite:
            self.setUpSuite()

        paramIndex = 0
        for test in self:
            if result.shouldStop:
                break

            if unittest.suite._isnotsuite(test):
                self._tearDownPreviousClass(test, result)
                self._handleModuleFixture(test, result)
                self._handleClassSetUp(test, result)
                result._previousTestClass = test.__class__

                if (getattr(test.__class__, '_classSetupFailed', False) or
                        getattr(result, '_moduleSetUpFailed', False)):
                    continue

            if not debug:
                test._param = self._params[paramIndex]
                paramIndex += 1
                test(result)
                test._param = None
            else:
                test.debug()

        if topLevel:
            self._tearDownPreviousClass(None, result)
            self._handleModuleTearDown(result)
            result._testRunEntered = False

        if self.tearDownSuite:
            self.tearDownSuite()
        return result

import pdb
import random
import logging
import copy
import sys
import re
import os
import inspect
import json
#import __builtin__
import unittest
import YTest

def skipIfNotRoot(func):
    def func_wrapper(self):
        if os.geteuid() != 0:
            self.skipTest('root privilege required')
        else:
            func(self)
    return func_wrapper

def skip_common(func):
    def __skip_common_wrap(self):
        setUp_funcname = re.sub("^test_", "setUp_", self.getTestName())
        tearDown_funcname = re.sub("^test_", "tearDown_", self.getTestName())
        su_cond = setUp_funcname in self.skip_common_list
        td_cond = tearDown_funcname in self.skip_common_list
        if not (su_cond and td_cond):
            err_msg = 'You must declared @skip_common to ' \
                      + setUp_funcname + ', ' + tearDown_funcname + '.'
            raise Exception(err_msg)
        else:
            func(self)
    return __skip_common_wrap

class CallFuncNotFound(Exception):
    pass

class TestBase(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestBase, self).__init__(*args, **kwargs)
        self.skip_common_list = self._getSkipCommonFunc()

    def _callFunc(self, funcName, beforeMarker=None, afterMarker=None):
        if hasattr(self, funcName):
            func = getattr(self, funcName)
            if beforeMarker:
                logging.info(beforeMarker)
            ret = func()
            if afterMarker:
                logging.info(afterMarker)
            return ret
        else:
            raise CallFuncNotFound

    def _getSkipCommonFunc(self):
        members = inspect.getmembers(self)
        _skipped_members = []
        for member in members:
            if (hasattr(member[1], '__name__') and
               member[1].__name__ is '__skip_common_wrap'):
                _skipped_members.append(member[0])
        return _skipped_members

    def getTestName(self, longName=False):
        return self.id() if longName else self.id().split(".")[-1]

    def setUp(self):
        try:
            ytestConfig = __builtin__.ytestConfig
        except:
            ytestConfig = {'randSeed': None, 'param': None}

        seed = ytestConfig['randSeed']
        if not seed:
            seed = random.randint(0, 0xffffffff)
        random.seed(seed)
        logging.info("-" * 48)
        logging.info("Start %s, Random Seed = %d" % (self.getTestName(True), seed))

        # override self._param if given from the ytest command line
        if ytestConfig['param']:
            self._param = json.loads(ytestConfig['param'])

        if not hasattr(self, '_param'):
            self._param = None

        funcName = re.sub("^test_", "setUp_", self.getTestName())

        if hasattr(self, funcName):
            # call only setUp_SubTest for subtest with @skip_common decorator
            if funcName in self.skip_common_list:
                try:
                    self._callFunc(funcName,
                                   '>>> ' + funcName + '()',
                                   '<<< ' + funcName + '()')
                except CallFuncNotFound:
                    pass
            # call setUpCommon & setUp_SubTest
            else:
                try:
                    self._callFunc('setUpCommon',
                                   '>>> ' + 'setUpCommon' + '()',
                                   '<<< ' + 'setUpCommon' + '()')
                    self._callFunc(funcName,
                                   '>>> ' + funcName + '()',
                                   '<<< ' + funcName + '()')
                except CallFuncNotFound:
                    pass
        else:
            # call only setUpCommon
            try:
                self._callFunc('setUpCommon',
                               '>>> ' + 'setUpCommon' + '()',
                               '<<< ' + 'setUpCommon' + '()')
            except CallFuncNotFound:
                pass

    def tearDown(self):
        funcName = re.sub("^test_", "tearDown_", self.getTestName())

        if hasattr(self, funcName):
            # call only tearDown_SubTest for subtest with @skip_common decorator
            if funcName in self.skip_common_list:
                try:
                    self._callFunc(funcName,
                                   '>>> ' + funcName + '()',
                                   '<<< ' + funcName + '()')
                except CallFuncNotFound:
                    pass
            # call tearDownCommon & tearDown_SubTest
            else:
                try:
                    self._callFunc(funcName,
                                   '>>> ' + funcName + '()',
                                   '<<< ' + funcName + '()')
                    self._callFunc('tearDownCommon',
                                   '>>> ' + 'tearDownCommon' + '()',
                                   '<<< ' + 'tearDownCommon' + '()')
                except CallFuncNotFound:
                    pass
        else:
            # call only tearDownCommon
            try:
                self._callFunc('tearDownCommon',
                               '>>> ' + 'tearDownCommon' + '()',
                               '<<< ' + 'tearDownCommon' + '()')
            except CallFuncNotFound:
                pass

        logging.info("Finish %s" % (self.getTestName(True)))

    def skipTest(self, reason):
        logging.warning('Skip ' + self.getTestName() + ', ' + reason)
        super(TestBase, self).skipTest(reason)

    def setUpCommon(self):
        pass

    def tearDownCommon(self):
        pass

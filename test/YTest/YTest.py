import pdb
import os
import sys
import imp
import re


# Decorator to add test case to a list
def TestCaseExt(cls):
    def wrap(func):
        assert func.__name__.startswith('test_') or \
               func.__name__.startswith('setUp_') or \
               func.__name__.startswith('tearDown_')
        setattr(cls, func.__name__, func)

    return wrap


# Decorator to add test suite to a list
def TestSuiteExt(mod):
    def wrap(func):
        assert func.__name__.startswith('suite_')
        setattr(mod, func.__name__, func)

    return wrap


def RegisterTestCases(cls, lst):
    for elem in lst:
        setattr(cls, elem[0], elem[1])


def RegisterTestSuites(mod, lst):
    for elem in lst:
        setattr(mod, elem[0], elem[1])


# Decorator to append method to class
def ClassExt(cls):
    def wrap(func):
        setattr(cls, func.__name__, func)
        # setattr(cls, func.__name__, classmethod(func))

    return wrap


# Load plugin modules for given test name
def LoadPlugins(moduleName, *args, **kwargs):
    pluginPattern = '.*_' + moduleName.split('.')[-1] + 'Plugin([\-_].+)*.py$'
    for p in sys.path:
        for root, dirs, files in os.walk(p):
            for file in files:
                if re.match(pluginPattern, file):
                    filePath = os.path.join(root, file)
                    mod = imp.load_source('', filePath)
                    if hasattr(mod, 'initPlugin'):
                        mod.initPlugin(*args, **kwargs)


def GetClassByName(cls):
    parts = cls.split('.')
    partsCopy = parts[:]
    while partsCopy:
        try:
            module = __import__('.'.join(partsCopy))
            break
        except ImportError:
            del partsCopy[-1]
            if not partsCopy:
                raise
    parts = parts[1:]
    obj = module
    for part in parts:
        parent, obj = obj, getattr(obj, part)
    return obj

#!/usr/bin/env python

import pdb
import sys
import os
import unittest
#import __builtin__
import optparse
import logging
import json
import pprint
from YTest.YTestResult import YTestResult

def testCaseListGen(s):
  paramIndex = 0
  for test in s:
    param = None
    if hasattr(s, '_params'):
      param = s._params[paramIndex]
      paramIndex += 1
    if unittest.suite._isnotsuite(test):
      yield test, param
    else:
      for t in testCaseListGen(test):
        yield t

def main():
  usage = "Usage: %prog [options] TestCase[, TestCase ...]"
  parser = optparse.OptionParser(usage=usage)
  parser.add_option("-p", "--searchPath",
                    action="append", type="string", dest="searchPath",
                    help="specify search directory")
  parser.add_option("-r", "--randSeed",
                    action="store", type="int", dest="randSeed",
                    help="initialize random seed")
  parser.add_option("--param",
                    action="store", type="string", dest="param",
                    help="test parameters in json format")
  parser.add_option("-l", "--list",
                    action="store_true", dest="listTestCases",
                    help="list test cases")
  parser.add_option("--reportUrl",
                    action="store", type="string", dest="reportUrl",
                    help="URL to report results")
  parser.add_option("-v", "--verbose",
                    action="store_true", dest="verbose",
                    help="verbose output")

  options, args = parser.parse_args()

  logLevel = logging.INFO
  loggingFormat = '%(asctime)-15s %(levelname)-7s %(message)s'
  if options.verbose:
    logLevel = logging.DEBUG
    loggingFormat = '%(asctime)-15s %(levelname)-7s ' + \
                    '{%(module)s:%(lineno)d} %(message)s'

  logger = logging.getLogger()
  logger.setLevel(logLevel)
  formatter = logging.Formatter(loggingFormat)
  stdoutHandler = logging.StreamHandler(sys.stdout)
  stdoutHandler.setFormatter(formatter)
  logger.addHandler(stdoutHandler)

  ytestConfig = { 'randSeed':options.randSeed,
                  'param':options.param }
  #__builtin__.ytestConfig = ytestConfig

  # initialize search dirs
  if not options.searchPath:
    options.searchPath = []
  for d in options.searchPath:
    d = os.path.abspath(d)
    if not d in sys.path:
      sys.path.insert(0, d)

  # load test cases
  loader = unittest.TestLoader()
  testNames = []
  for arg in args:
      if arg.endswith('.py'):
          testNames.append(arg[:-3])
      else:
          testNames.append(arg)
  suite = loader.loadTestsFromNames(testNames)

  if options.listTestCases:
    userParam = None if not options.param else json.loads(options.param)
    tcList = []
    for tc in testCaseListGen(suite):
      tcList.append(tc)

    if options.listTestCases:
      result = []
      for tcTuple in tcList:
        tc    = tcTuple[0]
        param = userParam if userParam else tcTuple[1]
        testMethodStr = tc.__dict__['_testMethodName']
        testName = tc.__class__.__module__ + '.' + \
                   tc.__class__.__name__ + '.' + \
                   testMethodStr
        result.append((testName, param))
      pprint.pprint(result)
    return

  resultClass = None
  if options.reportUrl:
    resultClass = YTestResult
    resultClass.reportUrl = options.reportUrl

  unittest.installHandler()
  if suite.countTestCases() > 0:
    tRunner = unittest.TextTestRunner(failfast=True,
                                      resultclass=resultClass,
                                      verbosity=(2 if options.verbose else 0))
    tRunner.run(suite)
  else:
    parser.print_help()

if __name__ == "__main__":
  main()

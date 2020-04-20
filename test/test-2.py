#!/usr/bin/env python

import pdb
import objxp
from ObjXp import *

oTree = CreateObject(objxp.ObexTree)
oCb1 = CreateObject(objxp.ObexDefaultCallback, "Cb1")
oObj1 = CreateObject(objxp.ObexStringObject, "Hello")
oObj2 = CreateObject(objxp.ObexStringObject, "World")
oTree.registerCallback("/hardware/portInfo/et1", oCb1)
oTree.putObject("/hardware/portInfo/et1", oObj1)
oTree.putObject("/hardware/portInfo/et2", oObj2)

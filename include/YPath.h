#ifndef __YPATH_H__
#define __YPATH_H__

/*

---------------------------
YPath macro name definition
---------------------------

1. Start with "Yp" prefix

2. Use camel case
   #define YpHelloWorld "/path/to/hello/world"

3. Variable name rules:
  a. All capital letters

  b. Variables are delimited by double-underscore(__)
     #define YpProc__PROC__Cmd "/path/to/proc/${PROC}/cmd"

  c. Single-underscore is allowed in variable name but double(or more) is not.
     Good: #define YpProc__PROC_ID__Cmd  "/path/to/proc/${PROC_ID}/cmd"
     Bad:  #define YpProc__PROC__ID__Cmd "/path/to/proc/${PROC_ID}/cmd"

  d. Tailing delimiter must be removed
     Good: #define YpProc__PROCNAME   "/path/to/proc/${PROCNAME}"
     Bad:  #define YpProc__PROCNAME__ "/path/to/proc/${PROCNAME}"

  e. Use one delimiter between back-to-back variables
     Good: #define YpProc__GID__UID   "/path/to/proc/${GID}/${UID}
     Bad:  #define YpProc__GID____UID "/path/to/proc/${GID}/${UID}

----------------------------
YPath macro value definition
----------------------------
1. Variable format ${VAR_NAME=[DEFAULT_VALUE]}
   Examples)
   #define YpPathTo__FIRSTVAR__SECOND_VAR "/path/to/${FIRSTVAR}/${SECOND_VAR}"
   #define YpPathTo__THIRD_VAR "/path/to/${THIRD_VAR="hello"}

*/

#endif


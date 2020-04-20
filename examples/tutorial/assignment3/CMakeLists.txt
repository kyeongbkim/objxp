file (MAKE_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment3)

set (COMMON_SRCS ../AbcMfg.cpp  ../AbcSales.cpp  ../MfgOrder.cpp)

add_executable (AbcFactory3 ${COMMON_SRCS} AbcFactory.cpp)
target_include_directories(AbcFactory3 PRIVATE ..)
target_link_libraries (AbcFactory3
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})
set_target_properties (
    AbcFactory3 PROPERTIES
    OUTPUT_NAME AbcFactory
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment3)

add_executable (AbcHQ3 ${COMMON_SRCS} AbcHQ.cpp)
target_include_directories(AbcHQ3 PRIVATE ..)
target_link_libraries (AbcHQ3
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})
set_target_properties (
    AbcHQ3 PROPERTIES
    OUTPUT_NAME AbcHQ
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment3)

add_executable (AbcDC3 ${COMMON_SRCS} AbcDC.cpp)
target_include_directories(AbcDC3 PRIVATE ..)
target_link_libraries (AbcDC3
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})
set_target_properties (
    AbcDC3 PROPERTIES
    OUTPUT_NAME AbcDC
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment3)

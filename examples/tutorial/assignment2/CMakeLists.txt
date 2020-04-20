file (MAKE_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment2)

set (COMMON_SRCS ../AbcMfg.cpp  ../AbcSales.cpp  ../MfgOrder.cpp)

add_executable (AbcFactory2 ${COMMON_SRCS} AbcFactory.cpp)
target_include_directories(AbcFactory2 PRIVATE ..)
target_link_libraries (AbcFactory2
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})
set_target_properties (
    AbcFactory2 PROPERTIES
    OUTPUT_NAME AbcFactory
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment2)

add_executable (AbcHQ2 ${COMMON_SRCS} AbcHQ.cpp)
target_include_directories(AbcHQ2 PRIVATE ..)
target_link_libraries (AbcHQ2
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})
set_target_properties (
    AbcHQ2 PROPERTIES
    OUTPUT_NAME AbcHQ
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment2)

file (MAKE_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment1)

set (COMMON_SRCS ../AbcMfg.cpp  ../AbcSales.cpp  ../MfgOrder.cpp)

add_executable (AbcCompany1 ${COMMON_SRCS} AbcCompany.cpp)
target_include_directories(AbcCompany1 PRIVATE ..)
target_link_libraries (AbcCompany1
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})
set_target_properties (
    AbcCompany1 PROPERTIES
    OUTPUT_NAME AbcCompany
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR}/tutorial/assignment1)

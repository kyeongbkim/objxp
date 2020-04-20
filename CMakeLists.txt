cmake_minimum_required (VERSION 3.5.1)

set (PYTHON_SUPPORT $ENV{PYTHON_SUPPORT})
if ("${PYTHON_SUPPORT}" STREQUAL "")
    set (PYTHON_SUPPORT "yes")
endif ()

if ("${PYTHON_SUPPORT}" STREQUAL "yes")
    if (DEFINED ENV{PYTHON_VERSION})
        set (PYTHON_VERSION $ENV{PYTHON_VERSION})
    else ()
        set (PYTHON_VERSION "3")
    endif ()
    message (STATUS "PYTHON_VERSION = " ${PYTHON_VERSION})
endif ()

set (EXTRA_INC_DIRS $ENV{EXTRA_INC_DIRS})

set (EXTRA_LIB_SRCS $ENV{EXTRA_LIB_SRCS})

if (NOT "$ENV{CROSS_COMPILE}" STREQUAL "")
    set (CMAKE_C_COMPILER $ENV{CROSS_COMPILE}gcc)
    set (CMAKE_CXX_COMPILER $ENV{CROSS_COMPILE}g++)
endif ()

#-------------------------------------------------------------------------------

project (objxp)

include_directories (include ${EXTRA_INC_DIRS})

set (LIB_SRCS
    src/ObexServer.cpp
    src/ObexClient.cpp
    src/Bitset.cpp
    src/TimerObject.cpp
    src/IoObject.cpp
    src/MsgIo.cpp
    src/BasicSocketAcceptor.cpp
    src/TcpSocketAcceptor.cpp
    src/UnixSocketAcceptor.cpp
    src/SocketConnector.cpp
    src/TcpSocketConnector.cpp
    src/UnixSocketConnector.cpp
    src/ConsoleIo.cpp
    src/ObexNumberObject.cpp
    src/ObexStringObject.cpp
    src/ObexJsonObject.cpp
    src/ObexTree.cpp
    src/ObexCallbackReceiver.cpp
    src/ObexInfoProvider.cpp
    src/BasicIoService.cpp
    src/Yail.cpp
    src/YUtils.cpp
    src/c-intf.c
)

if (NOT "${EXTRA_LIB_SRCS}" STREQUAL "")
    list (APPEND LIB_SRCS ${EXTRA_LIB_SRCS})
endif()

set (LIB_NAME "objxp")
set (PROJECT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
set (PROJECT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
file (MAKE_DIRECTORY ${PROJECT_LIB_DIR})
file (MAKE_DIRECTORY ${PROJECT_BIN_DIR})
file (MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gen)

if ("${PYTHON_SUPPORT}" STREQUAL "yes")
    set (SWIG_INPUT_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/src/swig_template.i)
    set (SWIG_INPUT_I ${CMAKE_CURRENT_BINARY_DIR}/gen/lib${LIB_NAME}.i)
    set (SWIG_OUTPUT_CXX ${CMAKE_CURRENT_BINARY_DIR}/gen/lib${LIB_NAME}_wrap.cxx)
    set (SWIG_OUTPUT_PY ${CMAKE_CURRENT_BINARY_DIR}/gen/${LIB_NAME}.py)

    add_custom_command (
        OUTPUT ${SWIG_INPUT_I}
        COMMAND echo %module ${LIB_NAME} > ${SWIG_INPUT_I}
        COMMAND echo >> ${SWIG_INPUT_I}
        COMMAND cat ${SWIG_INPUT_TEMPLATE} >> ${SWIG_INPUT_I}
        DEPENDS ${SWIG_INPUT_TEMPLATE}
    )
    add_custom_command (
        OUTPUT ${SWIG_OUTPUT_CXX} ${SWIG_OUTPUT_PY}
        COMMAND swig -o ${SWIG_OUTPUT_CXX}
                     -I${CMAKE_CURRENT_SOURCE_DIR}/include
                     -c++ -python ${SWIG_INPUT_I}
        DEPENDS ${SWIG_INPUT_I}
    )

    list (APPEND LIB_SRCS src/ObexPythonCallback.cpp ${SWIG_OUTPUT_CXX})

    execute_process (
        COMMAND python${PYTHON_VERSION}-config --includes
        OUTPUT_VARIABLE PYTHON_CFLAGS
    )
    string (REPLACE "\n" " " PYTHON_CFLAGS ${PYTHON_CFLAGS})
    string (REPLACE " " ";" PYTHON_CFLAGS ${PYTHON_CFLAGS})

    execute_process (
        COMMAND python${PYTHON_VERSION}-config --ldflags --libs
        OUTPUT_VARIABLE PYTHON_LDFLAGS
    )
    string (REPLACE "\n" " " PYTHON_LDFLAGS ${PYTHON_LDFLAGS})
    string (REPLACE " " ";" PYTHON_LDFLAGS ${PYTHON_LDFLAGS})

    set (BOOST_PYTHON boost_python${PYTHON_VERSION})
endif ()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_compile_options (-Wno-deprecated-declarations
                    -DRAPIDJSON_HAS_STDSTRING=1
                    ${PYTHON_CFLAGS})

add_library (${LIB_NAME} SHARED ${LIB_SRCS})
target_link_libraries(${LIB_NAME} ${BOOST_PYTHON} rt)
set_target_properties (${LIB_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_LIB_DIR})

if ("${PYTHON_SUPPORT}" STREQUAL "yes")
    add_custom_command(TARGET ${LIB_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/gen/${LIB_NAME}.py
                ${PROJECT_LIB_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/src/ObjXp.py
                ${PROJECT_LIB_DIR}/
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                lib${LIB_NAME}.so
                ${PROJECT_LIB_DIR}/_${LIB_NAME}.so
    )
endif ()

if ("$ENV{BUILD_EXAMPLES}" STREQUAL "yes")
    message (STATUS "Build examples = $ENV{BUILD_EXAMPLES}")
    add_subdirectory (examples)
endif ()

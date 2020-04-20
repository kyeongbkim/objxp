if ("${PYTHON_SUPPORT}" STREQUAL "yes")
    list (APPEND PYTHON_LDFLAGS boost_python${PYTHON_VERSION})
endif ()
list (APPEND PYTHON_LDFLAGS rt)

add_executable (HelloYail HelloYail.cpp)
target_link_libraries (HelloYail
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})

add_executable (RefCount RefCount.cpp)
target_link_libraries (RefCount
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})

add_executable (RawPointer RawPointer.cpp)
target_link_libraries (RawPointer
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})

add_executable (PointerCasting PointerCasting.cpp)
target_link_libraries (PointerCasting
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})

add_executable (DemoListener DemoListener.cpp)
target_link_libraries (DemoListener
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})

add_executable (ObexTest ObexTest.cpp)
target_link_libraries (ObexTest
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})

add_executable (Yserv Yserv.cpp)
target_link_libraries (Yserv
    ${LIB_NAME} ${PYTHON_LDFLAGS} ${EXTRA_LDFLAGS})

set_target_properties (
    HelloYail RefCount RawPointer PointerCasting DemoListener ObexTest Yserv
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR})

if ("$ENV{BUILD_TUTORIAL}" STREQUAL "yes")
    message (STATUS "Build tutorial = $ENV{BUILD_TUTORIAL}")
    add_subdirectory (tutorial)
endif ()

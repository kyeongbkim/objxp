add_subdirectory (assignment1)
add_subdirectory (assignment2)
add_subdirectory (assignment3)

if ("${PYTHON_SUPPORT}" STREQUAL "yes")
    add_custom_target (CopyUtils ALL)

    add_custom_command (
        TARGET CopyUtils POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/utils/putStringObject.py
                ${PROJECT_BIN_DIR}/putStringObject.py
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/utils/putStringObject.py
    )

    add_custom_command (
        TARGET CopyUtils POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/utils/delObject.py
                ${PROJECT_BIN_DIR}/delObject.py
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/utils/delObject.py
    )
endif ()


set(TYPOON_IMPORTED_SRCS
)

typoon_add_executable(UnitTests
    SRCS ${TYPOON_IMPORTED_SRCS}
    NO_GROUP_SOURCES ${TYPOON_IMPORTED_SRCS}
)

target_link_libraries(UnitTests
    PRIVATE
        Imm32.lib
)

source_group(
    TREE "${TYPOON_SRC_DIR}"
    PREFIX "Imported Sources"
    FILES ${TYPOON_IMPORTED_SRCS}
)

target_include_directories(UnitTests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

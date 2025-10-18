set(TYPOON_IMPORTED_SRCS
    ${TYPOON_SRC_DIR}/imm/composition.cpp
    ${TYPOON_SRC_DIR}/imm/imm_simulator.cpp
    ${TYPOON_SRC_DIR}/input_multicast/input_multicast.cpp
    ${TYPOON_SRC_DIR}/match/trigger_tree.cpp
    ${TYPOON_SRC_DIR}/match/trigger_trees_per_program.cpp
    ${TYPOON_SRC_DIR}/parse/parse_match.cpp
    ${TYPOON_SRC_DIR}/utils/string.cpp
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

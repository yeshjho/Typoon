set(TYPOON_IMPORTED_SRCS
    ${TYPOON_SRC_DIR}/ime/Composition.cpp
    ${TYPOON_SRC_DIR}/logger/Logger.cpp
    ${TYPOON_SRC_DIR}/parse/Match.cpp
    ${TYPOON_SRC_DIR}/util/Hangeul.cpp
    ${TYPOON_SRC_DIR}/util/String.cpp
    ${TYPOON_SRC_DIR}/util/Unicode.cpp
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
    ${TYPOON_SRC_DIR}  # Typoon 내부에서의 include를 위해. UnitTests 안에선 Typoon/을 붙이도록 해야 함
)

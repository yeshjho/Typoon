typoon_add_executable(Typoon
    SRCS
        resource/Typoon.rc
)

target_link_libraries(Typoon
    PRIVATE
        reflect-cpp
        Imm32.lib
        gdiplus.lib
        dbghelp.lib
)

target_link_options(Typoon PRIVATE
    /SUBSYSTEM:WINDOWS
)

set(TYPOON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

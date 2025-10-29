typoon_add_library(reflect-cpp IS_EXTERNAL)

target_link_libraries(reflect-cpp
    PRIVATE
        yaml-cpp
)

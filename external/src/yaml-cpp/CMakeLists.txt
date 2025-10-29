typoon_add_library(yaml-cpp IS_EXTERNAL)

target_compile_definitions(yaml-cpp
    PRIVATE
        YAML_CPP_STATIC_DEFINE
)

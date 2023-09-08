#pragma once
#include <filesystem>

#include <json5/json5_input.hpp>
#include <json5/json5_output.hpp>
#include <json5/json5_reflect.hpp>


namespace json5::detail
{
    template <>
    inline error read(const value& in, std::filesystem::path& out)
    {
        if (in.is_string())
        {
            out = in.get_c_str();
            return { error::none };
        }

        return { error::string_expected };
    }

    template<>
    inline value write(writer& w, const std::filesystem::path& in)
    {
        return write(w, in.generic_string());
    }

}
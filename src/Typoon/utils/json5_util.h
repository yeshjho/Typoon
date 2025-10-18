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
            // If we don't cast it to char8_t, the locale comes in and the encoding gets messed up and it'll throw.
            out = reinterpret_cast<const char8_t*>(in.get_c_str());
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
#include "parse_keys.h"

#include <json5/json5_input.hpp>
#include <json5/json5_reflect.hpp>


namespace json5::detail
{

template<>
error read(const value& in, EKey& out)
{
    if (!in.is_string())
    {
        return { error::invalid_enum };
    }

    std::string inStr{ in.get_c_str() };
    std::ranges::transform(inStr, inStr.begin(), [](char c) { return std::tolower(c); });

    if (inStr == "a")
    {
        out = EKey::A;
    }
    else if (inStr == "b")
    {
        out = EKey::B;
    }
    else if (inStr == "c")
    {
        out = EKey::C;
    }
    else if (inStr == "d")
    {
        out = EKey::D;
    }
    else if (inStr == "e")
    {
        out = EKey::E;
    }
    else if (inStr == "f")
    {
        out = EKey::F;
    }
    else if (inStr == "g")
    {
        out = EKey::G;
    }
    else if (inStr == "h")
    {
        out = EKey::H;
    }
    else if (inStr == "i")
    {
        out = EKey::I;
    }
    else if (inStr == "j")
    {
        out = EKey::J;
    }
    else if (inStr == "k")
    {
        out = EKey::K;
    }
    else if (inStr == "l")
    {
        out = EKey::L;
    }
    else if (inStr == "m")
    {
        out = EKey::M;
    }
    else if (inStr == "n")
    {
        out = EKey::N;
    }
    else if (inStr == "o")
    {
        out = EKey::O;
    }
    else if (inStr == "p")
    {
        out = EKey::P;
    }
    else if (inStr == "q")
    {
        out = EKey::Q;
    }
    else if (inStr == "r")
    {
        out = EKey::R;
    }
    else if (inStr == "s")
    {
        out = EKey::S;
    }
    else if (inStr == "t")
    {
        out = EKey::T;
    }
    else if (inStr == "u")
    {
        out = EKey::U;
    }
    else if (inStr == "v")
    {
        out = EKey::V;
    }
    else if (inStr == "w")
    {
        out = EKey::W;
    }
    else if (inStr == "x")
    {
        out = EKey::X;
    }
    else if (inStr == "y")
    {
        out = EKey::Y;
    }
    else if (inStr == "z")
    {
        out = EKey::Z;
    }
    else if (inStr == " " || inStr == "space")
    {
        out = EKey::SPACE;
    }
    else if ()

    return { error::invalid_enum };
}

template<>
value write(writer& w, const EKey& in)
{
    return write(w, in.generic_string());
}

}

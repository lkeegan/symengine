#include <tinyxml2.h>
#include <symengine/parser/mathml_parser.h>
#include <symengine/real_double.h>
#include <symengine/real_mpfr.h>
#include <symengine/ntheory_funcs.h>
#include <symengine/parser.h>

namespace SymEngine
{

RCP<const Basic> parse_mathml(const std::string &xml)
{
    MathmlParser p{};
    return p.parse(xml);
}

RCP<const Basic> MathmlParser::parse(const std::string &xml)
{
    return integer(0);
}

MathmlParser::MathmlParser() = default;

} // namespace SymEngine

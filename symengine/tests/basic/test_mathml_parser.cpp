#include "catch.hpp"

#include <symengine/visitor.h>
#include <symengine/eval_double.h>
#include <symengine/parser.h>
#include <symengine/polys/basic_conversions.h>
#include <symengine/symengine_exception.h>
#include <symengine/parser/parser.h>

using SymEngine::Add;
using SymEngine::Basic;
using SymEngine::boolFalse;
using SymEngine::boolTrue;
using SymEngine::Complex;
using SymEngine::ComplexInf;
using SymEngine::down_cast;
using SymEngine::E;
using SymEngine::Eq;
using SymEngine::erf;
using SymEngine::erfc;
using SymEngine::from_basic;
using SymEngine::function_symbol;
using SymEngine::gamma;
using SymEngine::Ge;
using SymEngine::Gt;
using SymEngine::has_symbol;
using SymEngine::I;
using SymEngine::Inf;
using SymEngine::Integer;
using SymEngine::integer;
using SymEngine::is_a;
using SymEngine::Le;
using SymEngine::loggamma;
using SymEngine::logical_and;
using SymEngine::logical_nand;
using SymEngine::logical_nor;
using SymEngine::logical_not;
using SymEngine::logical_or;
using SymEngine::logical_xnor;
using SymEngine::logical_xor;
using SymEngine::Lt;
using SymEngine::make_rcp;
using SymEngine::max;
using SymEngine::min;
using SymEngine::minus_one;
using SymEngine::Mul;
using SymEngine::Ne;
using SymEngine::Number;
using SymEngine::one;
using SymEngine::parse_mathml;
using SymEngine::ParseError;
using SymEngine::pi;
using SymEngine::pow;
using SymEngine::Rational;
using SymEngine::rational;
using SymEngine::RCP;
using SymEngine::real_double;
using SymEngine::RealDouble;
using SymEngine::Symbol;
using SymEngine::symbol;
using SymEngine::UIntPoly;
using SymEngine::YYSTYPE;
using SymEngine::zero;

using namespace SymEngine::literals;

TEST_CASE("Mathml Parsing: internal data structures", "[mathml_parser]")
{
    std::string s;
    REQUIRE(eq(*parse_mathml(s), *integer(0)));
}
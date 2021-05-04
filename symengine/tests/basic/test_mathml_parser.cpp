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
using SymEngine::EulerGamma;
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
using SymEngine::mathml;
using SymEngine::max;
using SymEngine::min;
using SymEngine::minus_one;
using SymEngine::Mul;
using SymEngine::Ne;
using SymEngine::Number;
using SymEngine::one;
using SymEngine::parse;
using SymEngine::parse_mathml;
using SymEngine::ParseError;
using SymEngine::pi;
using SymEngine::piecewise;
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

TEST_CASE("Mathml parsing roundtrips", "[mathml_parser]")
{
    for (const std::string &str :
         {"1",
          "1.2",
          "1e-5",
          "pi",
          "E",
          "EulerGamma",
          "x",
          "x+y",
          "x+y+z",
          "x-y",
          "x-y-z",
          "x*y",
          "x*y*z",
          "X/Y",
          "X/Y/Z",
          "1/3",
          "1/3/2",
          "x**2",
          "x**2**3",
          "(x/3+y)**3",
          "a + c*(b*x)+y/z",
          "a + atanh(cos(1+c*(b*x))+sin(tan(y/z)))",
          "csc(2-cot(1+sec(x)))",
          "sinh(cosh(y)+tanh(qq))",
          "csch(2-coth(1+sech(x)))",
          "atan2(y, z)",
          "acos(asin(cos(atan(3) + sin(x+y))))",
          "atanh(acoth(x))+acsc(y*asin(z))+acos(w*asec(w*acsc(q)))",
          "asech(1+acot(asinh(x)+acsch(y)-acosh(z)+atanh(z))+acoth(z))",
          "log(x)",
          "ln(x)",
          "log(x, y)",
          "floor(x)",
          "ceiling(y)",
          "min(a, b)",
          "max(a, b)",
          "abs(x)",
          "x>3",
          "x>=3",
          "y<2",
          "y<=x",
          "y==q",
          "x<3 & y>2",
          "x<3 | y>2",
          "Xor(x<3 , y>2)",
          "Xnor(x<3 , y>2)",
          "Nor(x<3 , y>2)",
          "True",
          "False",
          "Not(True)",
          "Not(x<3)"}) {
        // parse infix
        auto expr = parse(str);
        // print as mathml
        auto xml = mathml(*expr);
        // parse mathml
        auto res = parse_mathml(xml);
        CAPTURE(str);
        CAPTURE(*expr);
        CAPTURE(xml);
        CAPTURE(*res);
        REQUIRE(eq(*expr, *res));
    }
}

TEST_CASE("Mathml parsing numbers", "[mathml_parser]")
{
    RCP<const Basic> res;
    RCP<const Basic> expr;

    res = parse_mathml("<cn>12345</cn>");
    expr = integer(12345);
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));

    res = parse_mathml("<cn type=\"rational\"> 12342 <sep/> 2342342 </cn>");
    expr = rational(12342, 2342342);
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));

    res = parse_mathml("<cn type=\"complex-cartesian\">3<sep/>4</cn>");
    expr = Complex::from_two_nums(*integer(3), *integer(4));
    REQUIRE(eq(*res, *expr));
    // todo: mathml prints a custom complex type
    // REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));
}

TEST_CASE("Mathml parsing constants", "[mathml_parser]")
{
    RCP<const Basic> res;
    RCP<const Basic> expr;

    res = parse_mathml("<true />");
    expr = boolTrue;
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));

    res = parse_mathml("<false />");
    expr = boolFalse;
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));

    res = parse_mathml("<pi />");
    expr = pi;
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));

    res = parse_mathml("<exponentiale />");
    expr = E;
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));

    res = parse_mathml("<eulergamma />");
    expr = EulerGamma;
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));
}

TEST_CASE("Mathml parsing piecewise", "[mathml_parser]")
{
    RCP<const Basic> res;
    RCP<const Basic> expr;

    auto x = symbol("x");
    auto y = symbol("y");
    auto int1 = interval(integer(1), integer(2), true, false);
    auto int2 = interval(integer(2), integer(5), true, false);
    auto int3 = interval(integer(5), integer(10), true, false);
    auto p = piecewise({{x, contains(x, int1)},
                        {y, contains(x, int2)},
                        {add(x, y), contains(x, int3)}});

    expr = p;
    res = parse_mathml(mathml(*expr));
    REQUIRE(eq(*res, *expr));
    REQUIRE(eq(*parse_mathml(mathml(*res)), *expr));
}
#include <iomanip>
#include <sstream>
#include "catch.hpp"
#include <tinyxml2.h>
#include <symengine/visitor.h>
#include <symengine/eval_double.h>
#include <symengine/parser.h>
#include <symengine/printers.h>
#include <symengine/symengine_exception.h>
#include <symengine/eval.h>

using SymEngine::Add;
using SymEngine::Basic;
using SymEngine::boolFalse;
using SymEngine::boolTrue;
using SymEngine::ComplexInf;
using SymEngine::down_cast;
using SymEngine::E;
using SymEngine::Eq;
using SymEngine::evalf;
using SymEngine::EvalfDomain;
using SymEngine::function_symbol;
using SymEngine::Ge;
using SymEngine::Gt;
using SymEngine::has_symbol;
using SymEngine::I;
using SymEngine::Inf;
using SymEngine::Integer;
using SymEngine::integer;
using SymEngine::is_a;
using SymEngine::Le;
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
using SymEngine::Nan;
using SymEngine::Ne;
using SymEngine::Number;
using SymEngine::one;
using SymEngine::parse_sbml;
using SymEngine::ParseError;
using SymEngine::pi;
using SymEngine::piecewise;
using SymEngine::pow;
using SymEngine::Rational;
using SymEngine::rational;
using SymEngine::RCP;
using SymEngine::real_double;
using SymEngine::RealDouble;
using SymEngine::sbml;
using SymEngine::Symbol;
using SymEngine::symbol;
using SymEngine::zero;

using namespace SymEngine::literals;

static double dbl(const Basic &res)
{
    return down_cast<const RealDouble &>(*evalf(res, 53, EvalfDomain::Real))
        .as_double();
}

TEST_CASE("Parsing: integers, basic operations", "[sbml_parser]")
{
    std::string s;
    RCP<const Basic> res;

    s = "-1^2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(-1)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "+1^2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(1)));

    s = "-2^2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(-4)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "-a^2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *neg(parse_sbml("a^2"))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "-3-5";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(-8)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "((3)+(1*0))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(3)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "((2))*(1+(2*3))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(14)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(1+1)*((1+1)+(1+1))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(8)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(1*3)*(2+4)*(2)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(36)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(1+3)/(2+4)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *div(integer(2), integer(3))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "2*3 + 50*2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(106)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "2^(3+2)+ 10";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(42)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "2^(3+2)+ 10";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(42)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(5^3)/8 + 12";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(div(integer(125), integer(8)), integer(12))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(5^3)/8 + 12";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(div(integer(125), integer(8)), integer(12))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "3*2+3-5+2/2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(5)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "4^2/2+2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(10)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "4^2/2+2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(10)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(1+2*(3+1)-5/(2+2))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(integer(9), div(integer(-5), integer(4)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "2 + -3";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(-1)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "2^-3*2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *div(one, integer(4))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "10000000000000000000000000";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *pow(integer(10), integer(25))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "0.123123123e-10";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "123123123123123.0";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "15%4";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    REQUIRE(dbl(*res) == 3.0);

    s = "-15%4";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    REQUIRE(dbl(*res) == -3.0);

    s = "(-15)%(-4)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    REQUIRE(dbl(*res) == -3.0);

    s = "(+15)%(-4)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    REQUIRE(dbl(*res) == 3.0);
}

TEST_CASE("Parsing: symbols", "[sbml_parser]")
{
    std::string s;
    RCP<const Basic> res;
    RCP<const Basic> x = symbol("x");
    RCP<const Basic> y = symbol("y");
    RCP<const Basic> w = symbol("w1");
    RCP<const Basic> l = symbol("l0ngn4me");

    s = "x + 2*y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(x, mul(integer(2), y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "w1*y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(w, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x^(3+w1)-2/y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(pow(x, add(integer(3), w)), div(integer(-2), y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "l0ngn4me - w1*y + 2^(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(add(l, neg(mul(w, y))), pow(integer(2), x))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "4*x/8 - (w1*y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(neg(mul(w, y)), div(x, integer(2)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "3*y + (2*y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(y, integer(5))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "3*y/(1+x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *div(mul(y, integer(3)), add(x, integer(1)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "y/x*x";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *y));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x * -y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(x, mul(y, integer(-1)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x ^ --y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *pow(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x^2e-1+3e+2-2e-2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(real_double(299.98), pow(x, real_double(0.2)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
}

TEST_CASE("Parsing: functions", "[sbml_parser]")
{
    std::string s;
    RCP<const Basic> res;
    RCP<const Basic> x = symbol("x");
    RCP<const Basic> y = symbol("y");
    RCP<const Basic> w = symbol("w");
    RCP<const Basic> z = symbol("z");

    s = "plus()";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *zero));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "plus(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *x));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "Plus(x, 3)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(x, integer(3))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "pluS(x, y, 2*z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(add(x, y), mul(integer(2), z))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "times()";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(1)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "times(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *x));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "times(x, 3)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(x, integer(3))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "times(x, y, 2*z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(mul(x, y), mul(integer(2), z))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "plus(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "Divide(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *div(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sin(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *sin(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    // all functions are case-insensitive: parser converts to lower case
    s = "Sin(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *sin(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "SiN(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *sin(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "SIN(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *sin(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "asin(-1)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *neg(div(pi, integer(2)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "arcsin(sin(x))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *asin(sin(x))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "ArcCot(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *acot(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "arccotH(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *acoth(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "y^(abs(sin(3) + x)) + sinh(2)";
    res = parse_sbml(s);
    REQUIRE(
        eq(*res, *add(pow(y, abs(add(sin(integer(3)), x))), sinh(integer(2)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "y^(abs(sin(3) + x)) + sinh(2)";
    res = parse_sbml(s);
    REQUIRE(
        eq(*res, *add(pow(y, abs(add(sin(integer(3)), x))), sinh(integer(2)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sin(asin(x)) + y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "ceil(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *ceiling(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "ceiling(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *ceiling(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "floor(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *floor(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "csc(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *csc(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    //    s = "delay(x, y)";
    //    res = parse_sbml(s);
    //    REQUIRE(eq(*res, *csc(x)));
    //    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "factorial(5)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(120)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "factorial(12)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *integer(479001600)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "factorial(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *gamma(add(integer(1), x))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "factorial(factorial(x)-1)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *gamma(gamma(add(integer(1), x)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "ln(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *log(x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "log10(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *(log(x, integer(10)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    // todo: log(x) should be configurable as log_10(x), ln(x), or to throw an
    // error
    s = "log(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *(log(x, integer(10)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "log(5, x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *(log(x, integer(5)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "piecewise(x, x>0)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *piecewise({{x, Gt(x, zero)}})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "piecewise(x, x>0, 0)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *piecewise({{x, Gt(x, zero)}, {zero, boolTrue}})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "piecewise(x, x>0, -x, x<=0)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *piecewise({{x, Gt(x, zero)},
                                 {mul(integer(-1), x), Le(x, zero)}})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "piecewise(x, x>0, -2*x, x<0, 6)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *piecewise({{x, Gt(x, zero)},
                                 {mul(integer(-2), x), Lt(x, zero)},
                                 {integer(6), boolTrue}})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "power(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *(pow(x, y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "root(y, x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *pow(x, div(integer(1), y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "root(3, x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *pow(x, div(integer(1), integer(3)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "root(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *pow(x, div(integer(1), integer(2)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sqr(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *pow(x, integer(2))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sqrT(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *pow(x, div(integer(1), integer(2)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "max(x, x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *max({x, y})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "max(x, y, max(x))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *max({x, y})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sin(max(log(y, x), min(x, y)))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *sin(max({log(x, y), min({x, y})}))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "and()";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolTrue));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    // throws: can't cast Symbol to Boolean apparently
    //        s = "and(x)";
    //        res = parse_sbml(s);
    //        REQUIRE(eq(*res, *x));
    //        REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    //    s = "and(x, y)";
    //    res = parse_sbml(s);
    //    CAPTURE(*res);
    //    REQUIRE(eq(*res, *logical_and({x, y})));
    //    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    //
    //    s = "and(x, y, z)";
    //    res = parse_sbml(s);
    //        REQUIRE(eq(*res, *logical_and({x, y, z})));
    //    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "and(TRUE)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({boolTrue})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "and(x<0)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Lt(x, zero)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "and(true, x>0)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({boolTrue, Gt(x, zero)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "and(x<4, x>0)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Lt(x, integer(4)), Gt(x, zero)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    //    // we want to replace x with the boolean condition x != 0
    //    // to replicate a C-style implicit cast-to-bool for integers
    //    s = "not(0)";
    //    res = parse_sbml(s);
    //    REQUIRE(eq(*res, *boolTrue));
    //    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    //
    //    s = "not(1)";
    //    res = parse_sbml(s);
    //    REQUIRE(eq(*res, *boolFalse));
    //    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
    //
    //    s = "not(x)";
    //    res = parse_sbml(s);
    //    REQUIRE(eq(*res, *Eq(x, integer(0))));
    //    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "not(x<4)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_not(Lt(x, integer(4)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "or()";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolFalse));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "or(true)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolTrue));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "or(true, false)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolTrue));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "or(false, false)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolFalse));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "or(x>2, y<x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_or({Gt(x, integer(2)), Lt(y, x)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor()";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolFalse));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor(x>=2, y<=x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_xor({Ge(x, integer(2)), Le(y, x)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor(true, false)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolTrue));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor(true, true)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolFalse));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    //    s = "eq(x, y)";
    //    res = parse_sbml(s);
    //    CAPTURE(*res);
    //    REQUIRE(eq(*res, eq(*x, *y)));
    //    CAPTURE(sbml(*res));
    //    CAPTURE(*parse_sbml(sbml(*res)));
    //
    //    //    s = "eq(x, y, z)";
    //    s = "x == y == z";
    //    res = parse_sbml(s);
    //    CAPTURE(*res);
    //    CAPTURE(sbml(*res));
    //    CAPTURE(*parse_sbml(sbml(*res)));
    //    // note: internally this is a set, so order is not defined
    //    // todo: see if they should compare equal in symengine or not
    //    //    REQUIRE(eq(*res, *Eq(x, y, z)));
    //    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x == y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Eq(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "neq(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Ne(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "geq(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Le(y, x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "geq(x, y, z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Le(y, x), Le(z, y)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "geq(x, y, z, w)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Le(y, x), Le(z, y), Le(w, z)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "gt(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Gt(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "gt(x, y, z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Gt(x, y), Gt(y, z)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "gt(x, y, z, w)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Gt(x, y), Gt(y, z), Gt(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "leq(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Le(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "leq(x, y, z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Le(x, y), Le(y, z)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "leq(x, y, z, w)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Le(x, y), Le(y, z), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "lt(x, y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Lt(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "lt(x, y, z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Lt(x, y), Lt(y, z)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x <= y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Le(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x >= y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Le(y, x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x > y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Lt(y, x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x < y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Lt(x, y)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x + y < x*y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Lt(add(x, y), mul(x, y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x + y >= x*y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Le(mul(x, y), add(x, y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x - y == x/y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Eq(sub(x, y), div(x, y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "x - y <= x/y";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Le(sub(x, y), div(x, y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(2*pi) > x";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Lt(x, mul(integer(2), pi))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sin(pi/2) == 1";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolTrue));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "log(10) > 2";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *boolFalse));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "ln(10/10) + sin(pi*2/2) + 3*x == -1";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Eq(mul(integer(3), x), minus_one)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "3*y/(1+x) > y/x*x";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Lt(y, div(mul(y, integer(3)), add(x, integer(1))))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(x < y) && (w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(x < y) || (w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_or({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "!(x < y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_not(Lt(x, y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor((x < y), (w >= z))";
    res = parse_sbml(s, false);
    REQUIRE(eq(*res, *logical_xor({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(x < y) && (w >= z) || (y == z)";
    res = parse_sbml(s);
    REQUIRE(
        eq(*res, *logical_or({logical_and({Lt(x, y), Le(z, w)}), Eq(y, z)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(x < y) && ((w >= z) || (y == z))";
    res = parse_sbml(s);
    REQUIRE(
        eq(*res, *logical_and({logical_or({Eq(y, z), Le(z, w)}), Lt(x, y)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "!(x < y) && (w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({logical_not(Lt(x, y)), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "!(x < y) || (w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_or({logical_not(Lt(x, y)), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor(!(x < y), (w >= z))";
    res = parse_sbml(s, false);
    REQUIRE(eq(*res, *logical_xor({logical_not(Lt(x, y)), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "(x < y) || xor((w >= z), (y == z))";
    res = parse_sbml(s, false);
    REQUIRE(
        eq(*res, *logical_or({Lt(x, y), logical_xor({Le(z, w), Eq(y, z)})})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor((x < y) && (w >= z), (y == z))";
    res = parse_sbml(s, false);
    REQUIRE(
        eq(*res, *logical_xor({logical_and({Lt(x, y), Le(z, w)}), Eq(y, z)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "and(x < y, w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_and({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "or(x < y, w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_or({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "!or(x < y, w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_nor({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "!and(x < y, w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_nand({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "xor(x < y, w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_xor({Lt(x, y), Le(z, w)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "!xor(x < y, w >= z)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_xnor({Lt(x, y), Ge(w, z)})));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "not(x < y)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *logical_not(Lt(x, y))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
}

TEST_CASE("Parsing: constants", "[sbml_parser]")
{
    std::string s;
    RCP<const Basic> res;
    RCP<const Basic> x = symbol("x");

    s = "pi*exponentiale";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(pi, E)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "2*pi";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(integer(2), pi)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "Avogadro";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *symbol("avogadro")));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "TIME";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *symbol("time")));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sin(pi/2)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *one));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "log(10)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *one));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "ln(1) + sin(pi*2/2) + 3*x";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *mul(integer(3), x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "inf";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Inf));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "infInIty";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Inf));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "nan";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Nan));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "notanumber";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *Nan));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
}

TEST_CASE("Parsing: function_symbols", "[sbml_parser]")
{
    std::string s;
    RCP<const Basic> res;
    RCP<const Basic> x = symbol("x");
    RCP<const Basic> y = symbol("y");
    RCP<const Basic> z = symbol("wt");

    s = "f(x)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *function_symbol("f", x)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "my_func(x, wt) + sin(f(y))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(function_symbol("my_func", {x, z}),
                          sin(function_symbol("f", y)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "func(x, y, wt) + f(sin(x))";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(function_symbol("func", {x, y, z}),
                          function_symbol("f", sin(x)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "f(g(2^x))";
    res = parse_sbml(s);
    REQUIRE(eq(
        *res, *function_symbol("f", function_symbol("g", pow(integer(2), x)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "f(g(2^x))";
    res = parse_sbml(s);
    REQUIRE(eq(
        *res, *function_symbol("f", function_symbol("g", pow(integer(2), x)))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
}

TEST_CASE("Parsing: multi-arg functions", "[sbml_parser]")
{
    std::string s;
    RCP<const Basic> res;
    RCP<const Basic> x1 = symbol("x1");
    RCP<const Basic> x2 = symbol("x2");

    s = "x1*pow(x2,-1)";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *div(x1, x2)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "z + f(x + y, g(x), h(g(x)))";
    res = parse_sbml(s);
    REQUIRE(sbml(*res) == s);
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
}

TEST_CASE("Parsing: doubles", "[sbml_parser]")
{
    std::string s;
    double d;
    RCP<const Basic> res;
    RCP<const Basic> x = symbol("x");

    s = "1.324";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *real_double(1.324)));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "0.0324*x + 2*3";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(mul(real_double(0.0324), x), integer(6))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "0.324e-1*x + 2*3";
    res = parse_sbml(s);
    REQUIRE(eq(*res, *add(mul(real_double(0.0324), x), integer(6))));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "1.324/(2+3)";
    res = parse_sbml(s);
    REQUIRE(is_a<RealDouble>(*res));
    d = down_cast<const RealDouble &>(*res).as_double();
    REQUIRE(std::abs(d - 0.2648) < 1e-12);
    // note: printing an expression containing a RealDouble & reparsing
    // does not always compare equal, as the stored double has ~17 significant
    // digits, but only 15 digits are printed.
    // first print & parse res again to get only 15 significant figures:
    res = parse_sbml(sbml(*res));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));

    s = "sqrt(2.0)+5";
    res = parse_sbml(s);
    REQUIRE(is_a<RealDouble>(*res));
    d = down_cast<const RealDouble &>(*res).as_double();
    REQUIRE(std::abs(d - (std::sqrt(2) + 5)) < 1e-12);
    // as above: first print to get doubles with 15 significant figures
    res = parse_sbml(sbml(*res));
    REQUIRE(eq(*res, *parse_sbml(sbml(*res))));
}

TEST_CASE("Parsing: errors", "[sbml_parser]")
{
    std::string s;

    s = "12x";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "x+y+";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "what()";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "x + (y))";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "x + max((3, 2+1)";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "2..33 + 2";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "(2)(3)";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "sin(x y)";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "max(,3,2)";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);

    s = "x+%y+z";
    CHECK_THROWS_AS(parse_sbml(s), ParseError &);
}

TEST_CASE("SBML mathml tests", "[sbml_parser]")
{
    std::string f = "/home/lkeegan/BIOQUANT/mathml-tests/tests/mathML_test_";
    for (int i = 250; i < 299; ++i) {
        std::stringstream ss;
        ss << f << std::setfill('0') << std::setw(4) << i << ".xml";
        std::string file = ss.str();
        CAPTURE(file);
        tinyxml2::XMLDocument doc;
        doc.LoadFile(file.c_str());
        doc.Print();
        auto *math{doc.FirstChildElement("math")};
        auto *anni{math->FirstChildElement("semantics")
                       ->FirstChildElement("annotation")};
        auto *anno{anni->NextSiblingElement("annotation")};
        REQUIRE(math != nullptr);
        std::string mi{anni->GetText()};
        std::string mo{anno->GetText()};
        std::cout << mi << std::endl;
        std::cout << mo << std::endl;

        RCP<const Basic> resi;
        RCP<const Basic> reso;
        resi = parse_sbml(mi);
        reso = parse_sbml(mo);

        CAPTURE(mi);
        CAPTURE(*resi);
        CAPTURE(mo);
        CAPTURE(*reso);
        CAPTURE(sbml(*resi));
        CAPTURE(*parse_sbml(sbml(*resi)));
        // input & output infix parse to the same expression
        REQUIRE(eq(*resi, *reso));
        // print & parse round trip also gives the same expression
        //        if(!eq(*resi, *ComplexInf)) {
        //            // skip complex inf, as we print that as "inf" in sbml
        //            REQUIRE(eq(*resi, *parse_sbml(sbml(*resi))));
        //        }
    }
}

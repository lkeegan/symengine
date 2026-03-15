#include "catch.hpp"

#include <symengine/logic.h>
#include <symengine/add.h>
#include <symengine/printers/codegen.h>
#include <symengine/sets.h>

using SymEngine::abs;
using SymEngine::acos;
using SymEngine::acosh;
using SymEngine::acoth;
using SymEngine::add;
using SymEngine::asin;
using SymEngine::asinh;
using SymEngine::atan;
using SymEngine::atan2;
using SymEngine::atanh;
using SymEngine::Basic;
using SymEngine::boolFalse;
using SymEngine::boolTrue;
using SymEngine::C89CodePrinter;
using SymEngine::C99CodePrinter;
using SymEngine::cbrt;
using SymEngine::ccode;
using SymEngine::ceiling;
using SymEngine::cos;
using SymEngine::cosh;
using SymEngine::cot;
using SymEngine::coth;
using SymEngine::cudacode;
using SymEngine::cudacode_float;
using SymEngine::CudaCodePrinter;
using SymEngine::CudaFloatCodePrinter;
using SymEngine::dummy;
using SymEngine::E;
using SymEngine::Eq;
using SymEngine::erf;
using SymEngine::erfc;
using SymEngine::exp;
using SymEngine::floor;
using SymEngine::gamma;
using SymEngine::Ge;
using SymEngine::Gt;
using SymEngine::Inf;
using SymEngine::Integer;
using SymEngine::integer;
using SymEngine::Interval;
using SymEngine::interval;
using SymEngine::jscode;
using SymEngine::JSCodePrinter;
using SymEngine::lambertw;
using SymEngine::Le;
using SymEngine::log;
using SymEngine::loggamma;
using SymEngine::logical_not;
using SymEngine::logical_xor;
using SymEngine::Lt;
using SymEngine::max;
using SymEngine::min;
using SymEngine::Nan;
using SymEngine::Ne;
using SymEngine::NegInf;
using SymEngine::pi;
using SymEngine::piecewise;
using SymEngine::rational;
using SymEngine::sign;
using SymEngine::sin;
using SymEngine::sinh;
using SymEngine::sqrt;
using SymEngine::symbol;
using SymEngine::tan;
using SymEngine::tanh;
using SymEngine::truncate;
using SymEngine::unevaluated_expr;

TEST_CASE("C-code printers", "[CodePrinter]")
{
    C89CodePrinter c89;
    C99CodePrinter c99;
    REQUIRE(c89.apply(Inf) == "HUGE_VAL");
    REQUIRE(c99.apply(Inf) == "INFINITY");
    REQUIRE(c89.apply(E) == "exp(1)");
    REQUIRE(c99.apply(E) == "exp(1)");
    REQUIRE(c89.apply(pi) == "acos(-1)");
    REQUIRE(c99.apply(pi) == "acos(-1)");
}

TEST_CASE("CUDA-code printers", "[CudaCodePrinter]")
{
    CudaCodePrinter cuda;
    CudaFloatCodePrinter cuda_float;
    auto x = symbol("x");
    auto y = symbol("y");
    auto z = symbol("z");
    auto p = add(add(add(add(x, mul(x, y)), pow(x, y)), cbrt(x)),
                 sqrt(integer(2)));

    REQUIRE(cuda.apply(Inf) == "CUDART_INF");
    REQUIRE(cuda.apply(E) == "exp(1.0)");
    REQUIRE(cuda.apply(pi) == "acos(-1.0)");
    REQUIRE(cuda.apply(Nan) == "CUDART_NAN");
    REQUIRE(cudacode(*integer(2)) == "2.0");
    REQUIRE(cudacode(*p) == "x + x*y + sqrt(2.0) + cbrt(x) + pow(x, y)");
    REQUIRE(cuda_float.apply(Inf) == "CUDART_INF_F");
    REQUIRE(cuda_float.apply(E) == "exp(1.0f)");
    REQUIRE(cuda_float.apply(pi) == "acos(-1.0f)");
    REQUIRE(cuda_float.apply(Nan) == "CUDART_NAN_F");
    REQUIRE(cudacode_float(*integer(2)) == "2.0f");
    REQUIRE(cudacode_float(*p) == "x + x*y + sqrt(2.0f) + cbrt(x) + pow(x, y)");

    p = gamma(x);
    REQUIRE(cudacode(*p) == "tgamma(x)");
    REQUIRE(cudacode_float(*p) == "tgamma(x)");

    p = loggamma(x);
    REQUIRE(cudacode(*p) == "lgamma(x)");
    REQUIRE(cudacode_float(*p) == "lgamma(x)");

    p = max({x, y, z});
    REQUIRE(cudacode(*p) == "fmax(x, fmax(y, z))");
    REQUIRE(cudacode_float(*p) == "fmax(x, fmax(y, z))");

    p = function_symbol("f", pow(integer(2), x));
    REQUIRE(cudacode(*p) == "f(pow(2.0, x))");
    REQUIRE(cudacode_float(*p) == "f(pow(2.0f, x))");

    p = sin(x);
    REQUIRE(cudacode(*p) == "sin(x)");
    REQUIRE(cudacode_float(*p) == "sin(x)");

    p = cot(x);
    REQUIRE(cudacode(*p) == "1.0/tan(x)");
    REQUIRE(cudacode_float(*p) == "1.0f/tan(x)");

    p = coth(x);
    REQUIRE(cudacode(*p) == "1.0/tanh(x)");
    REQUIRE(cudacode_float(*p) == "1.0f/tanh(x)");

    p = acoth(x);
    REQUIRE(cudacode(*p) == "atanh(1.0/x)");
    REQUIRE(cudacode_float(*p) == "atanh(1.0f/x)");

    CHECK_THROWS_AS(cudacode(*lambertw(x)), SymEngine::SymEngineException);
    CHECK_THROWS_AS(cudacode_float(*lambertw(x)),
                    SymEngine::SymEngineException);
}

TEST_CASE("Codegen boolean support", "[CodePrinter][CudaCodePrinter]")
{
    auto x = symbol("x");
    auto y = symbol("y");

    REQUIRE(ccode(*boolTrue) == "1.0");
    REQUIRE(cudacode(*boolFalse) == "0.0");
    REQUIRE(cudacode_float(*boolTrue) == "1.0f");

    auto xor_expr = logical_xor({Lt(x, integer(2)), Le(y, x)});
    REQUIRE(ccode(*xor_expr) == "(((x < 2) != 0) != ((y <= x) != 0))");
    REQUIRE(cudacode(*xor_expr) == "(((x < 2.0) != 0) != ((y <= x) != 0))");
    REQUIRE(cudacode_float(*xor_expr)
            == "(((x < 2.0f) != 0) != ((y <= x) != 0))");

    auto not_expr = logical_not(xor_expr);
    REQUIRE(ccode(*not_expr) == "!((((x < 2) != 0) != ((y <= x) != 0)))");
    REQUIRE(cudacode(*not_expr) == "!((((x < 2.0) != 0) != ((y <= x) != 0)))");
    REQUIRE(cudacode_float(*not_expr)
            == "!((((x < 2.0f) != 0) != ((y <= x) != 0)))");

    auto sign_expr = sign(x);
    REQUIRE(ccode(*sign_expr)
            == "((x == 0.0) ? (0.0) : ((x < 0.0) ? (-1.0) : (1.0)))");
    REQUIRE(cudacode(*sign_expr)
            == "((x == 0.0) ? (0.0) : ((x < 0.0) ? (-1.0) : (1.0)))");
    REQUIRE(cudacode_float(*sign_expr)
            == "((x == 0.0f) ? (0.0f) : ((x < 0.0f) ? (-1.0f) : (1.0f)))");

    auto uneval_expr = unevaluated_expr(add(x, y));
    REQUIRE(ccode(*uneval_expr) == "x + y");
    REQUIRE(cudacode(*uneval_expr) == "x + y");
    REQUIRE(cudacode_float(*uneval_expr) == "x + y");
}

TEST_CASE("Dummy", "[CodePrinter]")
{
    C89CodePrinter c89;
    auto foo1 = dummy("foo");
    auto foo2 = dummy("foo");
    REQUIRE(c89.apply(foo1) != c89.apply(foo2));
}
TEST_CASE("Arithmetic", "[ccode]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto p = add(add(add(add(x, mul(x, y)), pow(x, y)), mul(x, x)),
                 sqrt(integer(2)));
    REQUIRE(ccode(*p) == "x + x*y + sqrt(2) + pow(x, 2) + pow(x, y)");
}

TEST_CASE("Rational", "[ccode]")
{
    auto p = rational(1, 3);
    REQUIRE(ccode(*p) == "1.0/3.0");

    p = rational(1, -3);
    REQUIRE(ccode(*p) == "-1.0/3.0");
}

TEST_CASE("Functions", "[ccode]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto z = symbol("z");
    auto p = function_symbol("f", x);

    REQUIRE(ccode(*p) == "f(x)");

    p = function_symbol("f", pow(integer(2), x));
    REQUIRE(ccode(*p) == "f(pow(2, x))");

    p = abs(x);
    REQUIRE(ccode(*p) == "fabs(x)");
    p = sin(x);
    REQUIRE(ccode(*p) == "sin(x)");
    p = cos(x);
    REQUIRE(ccode(*p) == "cos(x)");
    p = tan(x);
    REQUIRE(ccode(*p) == "tan(x)");
    p = cot(x);
    REQUIRE(ccode(*p) == "1/tan(x)");
    p = atan2(x, y);
    REQUIRE(ccode(*p) == "atan2(x, y)");
    p = exp(x);
    REQUIRE(ccode(*p) == "exp(x)");
    p = log(x);
    REQUIRE(ccode(*p) == "log(x)");
    p = sinh(x);
    REQUIRE(ccode(*p) == "sinh(x)");
    p = cosh(x);
    REQUIRE(ccode(*p) == "cosh(x)");
    p = tanh(x);
    REQUIRE(ccode(*p) == "tanh(x)");
    p = coth(x);
    REQUIRE(ccode(*p) == "1/tanh(x)");
    p = asinh(x);
    REQUIRE(ccode(*p) == "asinh(x)");
    p = acosh(x);
    REQUIRE(ccode(*p) == "acosh(x)");
    p = atanh(x);
    REQUIRE(ccode(*p) == "atanh(x)");
    p = acoth(x);
    REQUIRE(ccode(*p) == "atanh(1/x)");
    p = floor(x);
    REQUIRE(ccode(*p) == "floor(x)");
    p = ceiling(x);
    REQUIRE(ccode(*p) == "ceil(x)");
    p = truncate(x);
    REQUIRE(ccode(*p) == "trunc(x)");
    p = erf(x);
    REQUIRE(ccode(*p) == "erf(x)");
    p = erfc(x);
    REQUIRE(ccode(*p) == "erfc(x)");
    p = gamma(x);
    REQUIRE(ccode(*p) == "tgamma(x)");
    p = loggamma(x);
    REQUIRE(ccode(*p) == "lgamma(x)");
    p = max({x, y, z});
    REQUIRE(ccode(*p) == "fmax(x, fmax(y, z))");
    p = min({x, y, z});
    REQUIRE(ccode(*p) == "fmin(x, fmin(y, z))");

    CHECK_THROWS_AS(ccode(*lambertw(x)), SymEngine::SymEngineException);
}

TEST_CASE("Relationals", "[ccode]")
{
    auto x = symbol("x");
    auto y = symbol("y");

    auto p = Eq(x, y);
    CHECK(ccode(*p) == "x == y");

    p = Ne(x, y);
    CHECK(ccode(*p) == "x != y");

    p = Le(x, y);
    CHECK(ccode(*p) == "x <= y");

    p = Lt(x, y);
    CHECK(ccode(*p) == "x < y");
}

TEST_CASE("Piecewise", "[ccode]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto int1 = interval(NegInf, integer(2), true, false);
    auto int2 = interval(integer(2), integer(5), true, false);
    auto p = piecewise({{x, contains(x, int1)},
                        {y, contains(x, int2)},
                        {add(x, y), boolTrue}});

    REQUIRE(ccode(*p)
            == "((x <= 2) ? (\n   x\n)\n: ((x > 2 && x <= 5) ? (\n   "
               "y\n)\n: (\n   x + y\n)))");

    REQUIRE(cudacode(*p)
            == "((x <= 2.0) ? (\n   x\n)\n: ((x > 2.0 && x <= 5.0) ? (\n   "
               "y\n)\n: (\n   x + y\n)))");
}

TEST_CASE("JavaScript math functions", "[jscode]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto z = symbol("z");
    auto p = function_symbol("f", x);

    REQUIRE(jscode(*p) == "f(x)");

    p = function_symbol("f", pow(integer(2), x));
    REQUIRE(jscode(*p) == "f(Math.pow(2, x))");

    p = sqrt(x);
    REQUIRE(jscode(*p) == "Math.sqrt(x)");
    p = cbrt(x);
    REQUIRE(jscode(*p) == "Math.cbrt(x)");
    p = abs(x);
    REQUIRE(jscode(*p) == "Math.abs(x)");
    p = sin(x);
    REQUIRE(jscode(*p) == "Math.sin(x)");
    p = cos(x);
    REQUIRE(jscode(*p) == "Math.cos(x)");
    p = max({x, y, z});
    REQUIRE(jscode(*p) == "Math.max(x, y, z)");
    p = min({x, y, z});
    REQUIRE(jscode(*p) == "Math.min(x, y, z)");
    p = exp(x);
    REQUIRE(jscode(*p) == "Math.exp(x)");
    JSCodePrinter JS;
    REQUIRE(JS.apply(pi) == "Math.PI");
}

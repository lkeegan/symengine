#include "catch.hpp"

#include <symengine/logic.h>
#include <symengine/add.h>
#include <symengine/printers/codegen.h>
#include <symengine/sets.h>

using SymEngine::abs;
using SymEngine::acos;
using SymEngine::acosh;
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
    REQUIRE(cuda_float.apply(Inf) == "CUDART_INF_F");
    REQUIRE(cuda_float.apply(E) == "exp(1.0f)");
    REQUIRE(cuda_float.apply(pi) == "acos(-1.0f)");
    REQUIRE(cuda_float.apply(Nan) == "CUDART_NAN_F");
}

TEST_CASE("Codegen boolean support", "[ccode][cudacode][cudacode_float]")
{
    auto x = symbol("x");
    auto y = symbol("y");

    REQUIRE(ccode(*boolTrue) == "1.0");
    REQUIRE(cudacode(*boolTrue) == "1.0");
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
TEST_CASE("Arithmetic", "[ccode][cudacode][cudacode_float]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto p = add(add(add(add(x, mul(x, y)), pow(x, y)), mul(x, x)),
                 sqrt(integer(2)));
    REQUIRE(ccode(*p) == "x + x*y + sqrt(2) + pow(x, 2) + pow(x, y)");
    REQUIRE(cudacode(*p) == "x + x*y + sqrt(2.0) + pow(x, 2.0) + pow(x, y)");
    REQUIRE(cudacode_float(*p) == "x + x*y + sqrt(2.0f) + pow(x, 2.0f) + pow(x, y)");
}

TEST_CASE("Rational", "[ccode][cudacode][cudacode_float]")
{
    auto p = rational(1, 3);
    REQUIRE(ccode(*p) == "1.0/3.0");
    REQUIRE(cudacode(*p) == "1.0/3.0");
    REQUIRE(cudacode_float(*p) == "1.0f/3.0f");

    p = rational(1, -3);
    REQUIRE(ccode(*p) == "-1.0/3.0");
    REQUIRE(cudacode(*p) == "-1.0/3.0");
    REQUIRE(cudacode_float(*p) == "-1.0f/3.0f");
}

TEST_CASE("Functions", "[ccode][cudacode][cudacode_float]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto z = symbol("z");
    auto p = function_symbol("f", x);

    REQUIRE(ccode(*p) == "f(x)");
    REQUIRE(cudacode(*p) == "f(x)");
    REQUIRE(cudacode_float(*p) == "f(x)");

    p = function_symbol("f", pow(integer(2), x));
    REQUIRE(ccode(*p) == "f(pow(2, x))");
    REQUIRE(cudacode(*p) == "f(pow(2.0, x))");
    REQUIRE(cudacode_float(*p) == "f(pow(2.0f, x))");

    p = abs(x);
    REQUIRE(ccode(*p) == "fabs(x)");
    REQUIRE(cudacode(*p) == "fabs(x)");
    REQUIRE(cudacode_float(*p) == "fabs(x)");
    p = sin(x);
    REQUIRE(ccode(*p) == "sin(x)");
    REQUIRE(cudacode(*p) == "sin(x)");
    REQUIRE(cudacode_float(*p) == "sin(x)");
    p = cos(x);
    REQUIRE(ccode(*p) == "cos(x)");
    REQUIRE(cudacode(*p) == "cos(x)");
    REQUIRE(cudacode_float(*p) == "cos(x)");
    p = tan(x);
    REQUIRE(ccode(*p) == "tan(x)");
    REQUIRE(cudacode(*p) == "tan(x)");
    REQUIRE(cudacode_float(*p) == "tan(x)");
    p = atan2(x, y);
    REQUIRE(ccode(*p) == "atan2(x, y)");
    REQUIRE(cudacode(*p) == "atan2(x, y)");
    REQUIRE(cudacode_float(*p) == "atan2(x, y)");
    p = exp(x);
    REQUIRE(ccode(*p) == "exp(x)");
    REQUIRE(cudacode(*p) == "exp(x)");
    REQUIRE(cudacode_float(*p) == "exp(x)");
    p = log(x);
    REQUIRE(ccode(*p) == "log(x)");
    REQUIRE(cudacode(*p) == "log(x)");
    REQUIRE(cudacode_float(*p) == "log(x)");
    p = sinh(x);
    REQUIRE(ccode(*p) == "sinh(x)");
    REQUIRE(cudacode(*p) == "sinh(x)");
    REQUIRE(cudacode_float(*p) == "sinh(x)");
    p = cosh(x);
    REQUIRE(ccode(*p) == "cosh(x)");
    REQUIRE(cudacode(*p) == "cosh(x)");
    REQUIRE(cudacode_float(*p) == "cosh(x)");
    p = tanh(x);
    REQUIRE(ccode(*p) == "tanh(x)");
    REQUIRE(cudacode(*p) == "tanh(x)");
    REQUIRE(cudacode_float(*p) == "tanh(x)");
    p = asinh(x);
    REQUIRE(ccode(*p) == "asinh(x)");
    REQUIRE(cudacode(*p) == "asinh(x)");
    REQUIRE(cudacode_float(*p) == "asinh(x)");
    p = acosh(x);
    REQUIRE(ccode(*p) == "acosh(x)");
    REQUIRE(cudacode(*p) == "acosh(x)");
    REQUIRE(cudacode_float(*p) == "acosh(x)");
    p = atanh(x);
    REQUIRE(ccode(*p) == "atanh(x)");
    REQUIRE(cudacode(*p) == "atanh(x)");
    REQUIRE(cudacode_float(*p) == "atanh(x)");
    p = floor(x);
    REQUIRE(ccode(*p) == "floor(x)");
    REQUIRE(cudacode(*p) == "floor(x)");
    REQUIRE(cudacode_float(*p) == "floor(x)");
    p = ceiling(x);
    REQUIRE(ccode(*p) == "ceil(x)");
    REQUIRE(cudacode(*p) == "ceil(x)");
    REQUIRE(cudacode_float(*p) == "ceil(x)");
    p = truncate(x);
    REQUIRE(ccode(*p) == "trunc(x)");
    REQUIRE(cudacode(*p) == "trunc(x)");
    REQUIRE(cudacode_float(*p) == "trunc(x)");
    p = erf(x);
    REQUIRE(ccode(*p) == "erf(x)");
    REQUIRE(cudacode(*p) == "erf(x)");
    REQUIRE(cudacode_float(*p) == "erf(x)");
    p = erfc(x);
    REQUIRE(ccode(*p) == "erfc(x)");
    REQUIRE(cudacode(*p) == "erfc(x)");
    REQUIRE(cudacode_float(*p) == "erfc(x)");
    p = gamma(x);
    REQUIRE(ccode(*p) == "tgamma(x)");
    REQUIRE(cudacode(*p) == "tgamma(x)");
    REQUIRE(cudacode_float(*p) == "tgamma(x)");
    p = loggamma(x);
    REQUIRE(ccode(*p) == "lgamma(x)");
    REQUIRE(cudacode(*p) == "lgamma(x)");
    REQUIRE(cudacode_float(*p) == "lgamma(x)");
    p = max({x, y, z});
    REQUIRE(ccode(*p) == "fmax(x, fmax(y, z))");
    REQUIRE(cudacode(*p) == "fmax(x, fmax(y, z))");
    REQUIRE(cudacode_float(*p) == "fmax(x, fmax(y, z))");
    p = min({x, y, z});
    REQUIRE(ccode(*p) == "fmin(x, fmin(y, z))");
    REQUIRE(cudacode(*p) == "fmin(x, fmin(y, z))");
    REQUIRE(cudacode_float(*p) == "fmin(x, fmin(y, z))");
}

TEST_CASE("Relationals", "[ccode][cudacode][cudacode_float]")
{
    auto x = symbol("x");
    auto y = symbol("y");

    auto p = Eq(x, y);
    CHECK(ccode(*p) == "x == y");
    CHECK(cudacode(*p) == "x == y");
    CHECK(cudacode_float(*p) == "x == y");

    p = Ne(x, y);
    CHECK(ccode(*p) == "x != y");
    CHECK(cudacode(*p) == "x != y");
    CHECK(cudacode_float(*p) == "x != y");

    p = Le(x, y);
    CHECK(ccode(*p) == "x <= y");
    CHECK(cudacode(*p) == "x <= y");
    CHECK(cudacode_float(*p) == "x <= y");

    p = Lt(x, y);
    CHECK(ccode(*p) == "x < y");
    CHECK(cudacode(*p) == "x < y");
    CHECK(cudacode_float(*p) == "x < y");
}

TEST_CASE("Piecewise", "[ccode][cudacode][cudacode_float]")
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

    REQUIRE(cudacode_float(*p)
            == "((x <= 2.0f) ? (\n   x\n)\n: ((x > 2.0f && x <= 5.0f) ? (\n   "
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

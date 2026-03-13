#include "catch.hpp"
#include "cudacode_runtime_config.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <nvrtc.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include <symengine/add.h>
#include <symengine/constants.h>
#include <symengine/functions.h>
#include <symengine/lambda_double.h>
#include <symengine/logic.h>
#include <symengine/pow.h>
#include <symengine/printers.h>
#include <symengine/sets.h>

using SymEngine::add;
using SymEngine::Basic;
using SymEngine::boolFalse;
using SymEngine::boolTrue;
using SymEngine::cbrt;
using SymEngine::contains;
using SymEngine::cudacode;
using SymEngine::cudacode_float;
using SymEngine::E;
using SymEngine::gamma;
using SymEngine::Ge;
using SymEngine::Gt;
using SymEngine::Inf;
using SymEngine::integer;
using SymEngine::interval;
using SymEngine::LambdaRealDoubleVisitor;
using SymEngine::loggamma;
using SymEngine::logical_and;
using SymEngine::logical_not;
using SymEngine::logical_or;
using SymEngine::logical_xor;
using SymEngine::Lt;
using SymEngine::max;
using SymEngine::mul;
using SymEngine::Nan;
using SymEngine::NegInf;
using SymEngine::pi;
using SymEngine::piecewise;
using SymEngine::pow;
using SymEngine::RCP;
using SymEngine::sign;
using SymEngine::sqrt;
using SymEngine::symbol;
using SymEngine::unevaluated_expr;
using SymEngine::vec_basic;

namespace
{

struct TestInput {
    double x;
    double y;
    double z;
};

struct RuntimeCase {
    std::string name;
    RCP<const Basic> expr;
    TestInput input;
    double double_tolerance;
    double float_tolerance;
};

enum class CudaScalarMode { Double, Float };

std::string build_kernel_source(const std::string &cuda_code,
                                const char *scalar_type)
{
    std::ostringstream source;
    source << "#include <cuda_runtime.h>\n";
    source << "#include <math_constants.h>\n";
    source << "\n";
    source << "extern \"C\" __global__ void evaluate(" << scalar_type << " x, "
           << scalar_type << " y, " << scalar_type << " z, " << scalar_type
           << " *out)\n";
    source << "{\n";
    source << "    *out = " << cuda_code << ";\n";
    source << "}\n";
    return source.str();
}

std::string compile_to_ptx(const std::string &source)
{
    nvrtcProgram program = nullptr;
    REQUIRE(nvrtcCreateProgram(&program, source.c_str(),
                               "symengine_cudacode.cu", 0, nullptr, nullptr)
            == NVRTC_SUCCESS);

    std::vector<std::string> options = {
        "--std=c++14",
        "--fmad=false",
    };

    std::stringstream stream(SYMENGINE_CUDA_INCLUDE_DIRS);
    std::string path;
    while (std::getline(stream, path, ';')) {
        if (not path.empty()) {
            options.push_back("--include-path=" + path);
        }
    }

    std::vector<const char *> option_ptrs;
    option_ptrs.reserve(options.size());
    for (const auto &option : options) {
        option_ptrs.push_back(option.c_str());
    }

    REQUIRE(nvrtcCompileProgram(program, static_cast<int>(option_ptrs.size()),
                                option_ptrs.data())
            == NVRTC_SUCCESS);

    size_t ptx_size = 0;
    REQUIRE(nvrtcGetPTXSize(program, &ptx_size) == NVRTC_SUCCESS);

    std::string ptx(ptx_size, '\0');
    REQUIRE(nvrtcGetPTX(program, &ptx[0]) == NVRTC_SUCCESS);
    REQUIRE(nvrtcDestroyProgram(&program) == NVRTC_SUCCESS);

    return ptx;
}

template <typename Scalar>
Scalar evaluate_on_device_typed(const std::string &cuda_code,
                                const TestInput &input, const char *scalar_type)
{
    REQUIRE(cuInit(0) == CUDA_SUCCESS);
    REQUIRE(cudaSetDevice(0) == cudaSuccess);
    const std::string ptx
        = compile_to_ptx(build_kernel_source(cuda_code, scalar_type));

    CUmodule module = nullptr;
    CUfunction function = nullptr;
    Scalar *device_output = nullptr;
    Scalar x = static_cast<Scalar>(input.x);
    Scalar y = static_cast<Scalar>(input.y);
    Scalar z = static_cast<Scalar>(input.z);
    void *kernel_args[] = {
        &x,
        &y,
        &z,
        &device_output,
    };
    Scalar out = 0;

    REQUIRE(cuModuleLoadDataEx(&module, ptx.c_str(), 0, nullptr, nullptr)
            == CUDA_SUCCESS);
    REQUIRE(cuModuleGetFunction(&function, module, "evaluate") == CUDA_SUCCESS);
    REQUIRE(
        cudaMalloc(reinterpret_cast<void **>(&device_output), sizeof(Scalar))
        == cudaSuccess);
    REQUIRE(cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, kernel_args,
                           nullptr)
            == CUDA_SUCCESS);
    REQUIRE(cuCtxSynchronize() == CUDA_SUCCESS);
    REQUIRE(
        cudaMemcpy(&out, device_output, sizeof(Scalar), cudaMemcpyDeviceToHost)
        == cudaSuccess);
    REQUIRE(cudaFree(device_output) == cudaSuccess);
    REQUIRE(cuModuleUnload(module) == CUDA_SUCCESS);
    return out;
}

double evaluate_on_device(const std::string &cuda_code, CudaScalarMode mode,
                          const TestInput &input)
{
    if (mode == CudaScalarMode::Double) {
        return evaluate_on_device_typed<double>(cuda_code, input, "double");
    }

    return static_cast<double>(
        evaluate_on_device_typed<float>(cuda_code, input, "float"));
}

double evaluate_on_host(const vec_basic &symbols, const Basic &expr,
                        const TestInput &input)
{
    LambdaRealDoubleVisitor visitor;
    visitor.init(symbols, expr);
    return visitor.call({input.x, input.y, input.z});
}

void require_codegen_match(const RuntimeCase &runtime_case,
                           const vec_basic &symbols, CudaScalarMode mode)
{
    const std::string cuda_code = mode == CudaScalarMode::Double
                                      ? cudacode(*runtime_case.expr)
                                      : cudacode_float(*runtime_case.expr);
    const double host_value
        = evaluate_on_host(symbols, *runtime_case.expr, runtime_case.input);
    const double device_value
        = evaluate_on_device(cuda_code, mode, runtime_case.input);

    INFO("cudacode: " << cuda_code);
    INFO("inputs: x=" << runtime_case.input.x << ", y=" << runtime_case.input.y
                      << ", z=" << runtime_case.input.z);
    INFO("host value: " << host_value);

    INFO("device value: " << device_value);
    if (std::isnan(host_value) || std::isnan(device_value)) {
        REQUIRE(std::isnan(host_value));
        REQUIRE(std::isnan(device_value));
        return;
    }
    if (std::isinf(host_value) || std::isinf(device_value)) {
        REQUIRE(std::isinf(host_value));
        REQUIRE(std::isinf(device_value));
        REQUIRE(std::signbit(host_value) == std::signbit(device_value));
        return;
    }

    const double scale = std::fmax(
        1.0, std::fmax(std::fabs(host_value), std::fabs(device_value)));
    REQUIRE(std::fabs(device_value - host_value)
            <= (mode == CudaScalarMode::Double ? runtime_case.double_tolerance
                                               : runtime_case.float_tolerance)
                   * scale);
}

} // namespace

TEST_CASE("CUDA code matches Lambda visitor", "[cuda][cudacode]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto z = symbol("z");
    auto a = add(x, z);

    vec_basic symbols = {x, y, z};

    auto arithmetic = add(add(add(add(x, mul(x, y)), pow(x, y)), cbrt(x)),
                          sqrt(integer(2)));
    auto max_expr = max({x, y, z});
    auto piecewise_expr = piecewise(
        {{x, contains(x, interval(NegInf, integer(2), true, false))},
         {y, contains(x, interval(integer(2), integer(5), true, false))},
         {add(x, y), boolTrue}});
    auto llvm_piecewise_expr = add(
        z, piecewise(
               {{x, contains(x, interval(NegInf, integer(2), true, false))},
                {y, contains(x, interval(integer(2), integer(5), true, false))},
                {z, Ge(x, integer(7))},
                {a, logical_and({Lt(x, integer(6)), Gt(x, integer(5))})},
                {add(x, y), boolTrue}}));

    std::vector<RuntimeCase> cases = {
        {"arithmetic", arithmetic, {3.0, 4.0, -2.0}, 1e-12, 5e-6},
        {"integer base power",
         pow(integer(2), x),
         {3.0, 4.0, -2.0},
         1e-12,
         1e-6},
        {"gamma", gamma(x), {3.0, 4.0, -2.0}, 1e-11, 1e-5},
        {"loggamma", loggamma(x), {3.0, 4.0, -2.0}, 1e-11, 1e-5},
        {"max", max_expr, {3.0, 4.0, -2.0}, 1e-12, 1e-6},
        {"piecewise lower branch",
         piecewise_expr,
         {1.0, 4.0, -2.0},
         1e-12,
         1e-6},
        {"piecewise middle branch",
         piecewise_expr,
         {3.0, 4.0, -2.0},
         1e-12,
         1e-6},
        {"piecewise fallback branch",
         piecewise_expr,
         {7.0, 4.0, -2.0},
         1e-12,
         1e-6},
        {"constant E", E, {3.0, 4.0, -2.0}, 1e-12, 1e-6},
        {"constant pi", pi, {3.0, 4.0, -2.0}, 1e-12, 1e-6},
        {"positive infinity", Inf, {3.0, 4.0, -2.0}, 1e-12, 1e-6},
        {"nan", Nan, {3.0, 4.0, -2.0}, 1e-12, 1e-6},
        {"bool true", boolTrue, {3.0, 4.0, -2.0}, 1e-12, 1e-6},
        {"bool false", boolFalse, {3.0, 4.0, -2.0}, 1e-12, 1e-6},
        {"logical and true",
         logical_and({Lt(x, integer(6)), Gt(x, integer(5))}),
         {5.5, 4.0, -2.0},
         1e-12,
         1e-6},
        {"logical and false",
         logical_and({Lt(x, integer(6)), Gt(x, integer(5))}),
         {4.0, 4.0, -2.0},
         1e-12,
         1e-6},
        {"logical or",
         logical_or({Lt(x, integer(2)), Gt(y, integer(5))}),
         {3.0, 6.0, -2.0},
         1e-12,
         1e-6},
        {"logical xor",
         logical_xor({Lt(x, integer(2)), Gt(y, integer(5))}),
         {3.0, 6.0, -2.0},
         1e-12,
         1e-6},
        {"logical not",
         logical_not(logical_xor({Lt(x, integer(2)), Gt(y, integer(5))})),
         {3.0, 4.0, -2.0},
         1e-12,
         1e-6},
        {"sign negative", sign(a), {-0.5, 4.0, -1.0}, 1e-12, 1e-6},
        {"sign zero", sign(a), {1.0, 4.0, -1.0}, 1e-12, 1e-6},
        {"sign positive", sign(a), {3.0, 4.0, -1.0}, 1e-12, 1e-6},
        {"unevaluated expr",
         unevaluated_expr(add(x, y)),
         {3.0, 4.0, -2.0},
         1e-12,
         1e-6},
        {"llvm piecewise ge branch",
         llvm_piecewise_expr,
         {7.0, 4.0, -1.0},
         1e-12,
         1e-6},
        {"llvm piecewise logical and branch",
         llvm_piecewise_expr,
         {5.5, 4.0, -1.0},
         1e-12,
         1e-6},
        {"llvm piecewise fallback branch",
         llvm_piecewise_expr,
         {6.5, 4.0, -1.0},
         1e-12,
         1e-6},
    };

    for (const auto mode : {CudaScalarMode::Double, CudaScalarMode::Float}) {
        for (const auto &runtime_case : cases) {
            DYNAMIC_SECTION(
                std::string(mode == CudaScalarMode::Double ? "double" : "float")
                + " " + runtime_case.name)
            {
                require_codegen_match(runtime_case, symbols, mode);
            }
        }
    }
}

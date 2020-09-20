#include <benchmark/benchmark.h>

#include <symengine/basic.h>
#include <symengine/add.h>
#include <symengine/symbol.h>
#include <symengine/dict.h>
#include <symengine/integer.h>
#include <symengine/mul.h>
#include <symengine/pow.h>

using SymEngine::Basic;
using SymEngine::Add;
using SymEngine::Mul;
using SymEngine::Pow;
using SymEngine::Symbol;
using SymEngine::symbol;
using SymEngine::integer;
using SymEngine::multinomial_coefficients;
using SymEngine::RCP;
using SymEngine::rcp_dynamic_cast;

void add1(benchmark::State &state){
    RCP<const Basic> x = symbol("x");
    RCP<const Basic> a, c;
    for (auto _ : state) {
        a = x;
        c = integer(1);
        for (int i = 0; i < state.range(0); i++) {
            a = add(a, mul(c, pow(x, integer(i))));
            c = mul(c, integer(-1));
        }
    }
    state.SetComplexityN(state.range(0));
    state.SetLabel("number of terms: " + std::to_string(rcp_dynamic_cast<const Add>(a)->get_dict().size()));
}

BENCHMARK(add1)->RangeMultiplier(2)->Range(4, 4096)->Complexity();

BENCHMARK_MAIN();

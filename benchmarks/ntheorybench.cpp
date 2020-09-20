#include <benchmark/benchmark.h>

#include <symengine/ntheory.h>

using SymEngine::mertens;
using SymEngine::mobius;
using SymEngine::prime_factor_multiplicities;
using SymEngine::integer;

void bench_mertens(benchmark::State &state){
    unsigned long r{0};
    long a{state.range(0)};
    for (auto _ : state) {
        r += mertens(a);
    }
    state.SetLabel("mertens("+std::to_string(a)+") = "+std::to_string(mertens(a)));
    state.SetComplexityN(a);
}

void bench_mobius(benchmark::State &state){
    int r{0};
    long a{state.range(0)};
    for (auto _ : state) {
        r += mobius(*integer(a));
    }
    state.SetLabel("mobius("+std::to_string(a)+") = "+std::to_string(mobius(*integer(a))));
    state.SetComplexityN(a);
}

void bench_prime_factor_multiplicities(benchmark::State &state){
    long a{state.range(0)};
    SymEngine::map_integer_uint primes_mul;
    for (auto _ : state) {
        primes_mul.clear();
        prime_factor_multiplicities(primes_mul, *integer(a));
    }
    state.SetComplexityN(a);
}

BENCHMARK(bench_mertens)->RangeMultiplier(2)->Range(1, 4096)->Complexity();
BENCHMARK(bench_mobius)->DenseRange(1, 150, 13)->Complexity();
BENCHMARK(bench_prime_factor_multiplicities)->DenseRange(17, 10000, 713)->Complexity();

BENCHMARK_MAIN();

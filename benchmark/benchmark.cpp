#include "../surf_paper/include/surf.hpp"
#include <fstream>
#include <string>
#include <benchmark/benchmark.h>
#include <random>
#include <algorithm>
#include "utils.h"
#include "benchmark_art.h"
#include "benchmark_btree.h"
#include "benchmark_surf_new.h"
#include "benchmark_surf_paper.h"
#include "benchmark_louds_dense_new.h"
#include "benchmark_louds_dense_paper.h"
#include "benchmark_louds_sparse_new.h"
#include "benchmark_louds_sparse_paper.h"
#include "benchmark_bloom.h"
#include "benchmark_louds.h"
#include <art.h>

const static auto MAX_N = 100000000;
const static auto N = 10000000;
using surf::SuffixType;
// BENCHMARK(BM_ConstructionBTree)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N)
//     ->Unit(benchmark::kMillisecond);
// BENCHMARK(BM_ConstructionSurf<suffix::Hash<8>>)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N/10)
//     ->Unit(benchmark::kMillisecond);
// BENCHMARK(BM_ConstructionSurfPaper<SuffixType::kNone>)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N/10)
//     ->Unit(benchmark::kMillisecond);
// BENCHMARK(BM_ConstructionSurfPaper<SuffixType::kHash, 8>)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N/10)
//     ->Unit(benchmark::kMillisecond);
// BENCHMARK(BM_ConstructionART)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N)
//     ->Unit(benchmark::kMillisecond);
// BENCHMARK(BM_ConstructionBloomFilter)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N)
//     ->Unit(benchmark::kMillisecond);
// BENCHMARK(BM_ConstructionLouds)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N)
//     ->Unit(benchmark::kMillisecond);




// BENCHMARK(BM_PointQueryBTree)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);
// BENCHMARK(BM_PointQuerySurf<suffix::Hash<8>>)
//     ->RangeMultiplier(10)
//     ->Range(N, N);
// BENCHMARK(BM_PointQuerySurfPaper<SuffixType::kNone>)
//     ->RangeMultiplier(10)
//     ->Range(N, N); // process gets killed if max range is MAX_N (Bug in SURF Paper)
// BENCHMARK(BM_PointQuerySurfPaper<SuffixType::kHash, 8>)
//     ->RangeMultiplier(10)
//     ->Range(N, N); // process gets killed if max range is MAX_N (Bug in SURF Paper)
// BENCHMARK(BM_PointQueryART)
//     ->RangeMultiplier(10)
//     ->Range(N, N*10);
// BENCHMARK(BM_PointQueryLouds)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);

// BENCHMARK(BM_PointQueryLoudsSparsePaper)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);
// BENCHMARK(BM_PointQueryLoudsSparse)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);

// BENCHMARK(BM_PointQueryLoudsDensePaper)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);
// BENCHMARK(BM_PointQueryLoudsDense)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);

// BENCHMARK(BM_AccessBitLoudsDensePaper)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);
// BENCHMARK(BM_AccessBitLoudsDense)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);

// BENCHMARK(BM_RankLoudsDensePaper)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);
// BENCHMARK(BM_RankLoudsDense)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);

// BENCHMARK(BM_AccessBitLoudsSparsePaper)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);
// BENCHMARK(BM_AccessBitLoudsSparse)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);
// BENCHMARK(BM_TraversalSurfPaper)
//     ->RangeMultiplier(10)
//     ->Range(N, MAX_N);

BENCHMARK(BM_FPRateSurf<1>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<2>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<3>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<4>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<5>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<6>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<7>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<8>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<9>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurf<10>)
    ->Ranges({{N, N},{50,50}});

BENCHMARK(BM_FPRateSurfPaper<1>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<2>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<3>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<4>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<5>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<6>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<7>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<8>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<9>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateSurfPaper<10>)
    ->Ranges({{N, N},{50,50}});


BENCHMARK(BM_FPRateBloomFilter<10>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<11>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<12>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<13>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<14>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<15>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<16>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<17>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<18>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<19>)
    ->Ranges({{N, N},{50,50}});
BENCHMARK(BM_FPRateBloomFilter<20>)
    ->Ranges({{N, N},{50,50}});

BENCHMARK_MAIN();

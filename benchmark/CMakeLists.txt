find_package(benchmark REQUIRED)
find_library(ART_LIBRARY art HINTS /usr/local/lib) # ART is required to be installed in your computer
find_library(TLX_LIBRARY tlx HINTS /usr/local/lib) # TLX is required to be installed in your computer
find_library(TX_LIBRARY tx HINTS /usr/local/lib) # TX is required to be installed in your computer

add_library(utils OBJECT utils.cpp)

add_executable(benchmark_art benchmark_art.cpp)
target_link_libraries(benchmark_art PUBLIC benchmark::benchmark)
target_link_libraries(benchmark_art PUBLIC ${ART_LIBRARY})   
target_link_libraries(benchmark_art PUBLIC utils)

add_executable(benchmark_bloom benchmark_bloom.cpp)
target_link_libraries(benchmark_bloom PUBLIC benchmark::benchmark)
target_link_libraries(benchmark_bloom PUBLIC utils)

add_executable(benchmark_btree benchmark_btree.cpp)
target_link_libraries(benchmark_btree PUBLIC benchmark::benchmark)
target_link_libraries(benchmark_btree PUBLIC ${TLX_LIBRARY}) 
target_link_libraries(benchmark_btree PUBLIC utils)

add_executable(benchmark_surf_new benchmark_surf_new.cpp)
target_link_libraries(benchmark_surf_new PUBLIC benchmark::benchmark)
target_link_libraries(benchmark_surf_new PUBLIC utils)

add_executable(benchmark_louds benchmark_louds.cpp)
target_link_libraries(benchmark_louds PUBLIC benchmark::benchmark)
target_link_libraries(benchmark_louds PUBLIC utils)
target_link_libraries(benchmark_louds PUBLIC ${TX_LIBRARY}) 

add_executable(benchmark_surf_paper benchmark_surf_paper.cpp)
target_link_libraries(benchmark_surf_paper PUBLIC benchmark::benchmark)
target_link_libraries(benchmark_surf_paper PUBLIC utils)

add_executable(benchmark_fst benchmark_fst.cpp)
target_link_libraries(benchmark_fst PUBLIC benchmark::benchmark)
target_link_libraries(benchmark_fst PUBLIC utils)

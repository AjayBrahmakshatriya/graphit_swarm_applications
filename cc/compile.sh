mkdir -p build
clang++ -o build/cc.o -c -std=c++14 -ftemplate-depth=900 -mllvm -regalloc=default -mllvm -swarm-abi=serial -O3 -gdwarf-3 -g -Wall -mno-avx -mno-avx2 -mno-sse4.2 -mfpmath=sse -fno-stack-protector -fswarm -fno-exceptions '-Rpass=(parallelizable-copy)|(profitability)|(bundling)|(fractalization)|(swarm-reductions)|(loop-coarsen)|(swarm-loop-expansion)' '-Rpass-missed=(parallelizable-copy)|(profitability)|(bundling)|(fractalization)|(swarm-reductions)|(loop-coarsen)|(swarm-loop-expansion)' '-Rpass-analysis=(parallelizable-copy)|(profitability)|(bundling)|(fractalization)|(swarm-reductions)|(loop-coarsen)|(swarm-loop-expansion)' -DSCC_RUNTIME=2 -DSCC_SERIAL_RUNTIME=1 -DPLS_APP_MAX_ARGS=2 -DSCC_HACKS=1 -DPLS_ALIGNMENT=64 -DPLS_CACHE_LINE=64 -DBFS_ISOLATE_DEPENDENCES_DELAY_ENQUEUE  -Iinclude -Iruntime/include cc.cpp
#clang++ -o pr.o -c -std=c++14 -ftemplate-depth=900 -mllvm -regalloc=default -mllvm -swarm-abi=serial -O3 -gdwarf-3 -g -Wall -mno-avx -mno-avx2 -mno-sse4.2 -mfpmath=sse -fno-stack-protector -fswarm -fno-exceptions '-Rpass=(parallelizable-copy)|(profitability)|(bundling)|(fractalization)|(swarm-reductions)|(loop-coarsen)|(swarm-loop-expansion)' '-Rpass-missed=(parallelizable-copy)|(profitability)|(bundling)|(fractalization)|(swarm-reductions)|(loop-coarsen)|(swarm-loop-expansion)' '-Rpass-analysis=(parallelizable-copy)|(profitability)|(bundling)|(fractalization)|(swarm-reductions)|(loop-coarsen)|(swarm-loop-expansion)' -DSCC_RUNTIME=2 -DSCC_SERIAL_RUNTIME=1 -DPLS_APP_MAX_ARGS=2 -DSCC_HACKS=1 -DPLS_ALIGNMENT=64 -DPLS_CACHE_LINE=64 -DBFS_ISOLATE_DEPENDENCES_DELAY_ENQUEUE -Iruntime/include -Iinclude pbfs/bfs.cpp


clang++ -o build/cc build/cc.o ~/scratch/swarm-graphit/swarm/benchmarks/build/release_scc_serial/sccrt/libsccrt.a

add_executable(order_book_benchmark order_book_benchmark.cpp)
target_link_libraries(order_book_benchmark PRIVATE MiniExchange::mini_exchange benchmark::benchmark_main)
mini_exchange_warnings(order_book_benchmark)
add_custom_target(run_benchmarks
    COMMAND $<TARGET_FILE:order_book_benchmark>
        --benchmark_min_time=0.1s --benchmark_repetitions=3
        --benchmark_out=${PROJECT_BINARY_DIR}/benchmark.json --benchmark_out_format=json
    DEPENDS order_book_benchmark
    USES_TERMINAL
    VERBATIM
)

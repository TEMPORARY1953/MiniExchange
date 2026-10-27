add_executable(mini_exchange_demo main.cpp)
target_link_libraries(mini_exchange_demo PRIVATE MiniExchange::mini_exchange)
mini_exchange_warnings(mini_exchange_demo)
add_custom_target(run_demo
    COMMAND $<TARGET_FILE:mini_exchange_demo>
    DEPENDS mini_exchange_demo
    USES_TERMINAL
    VERBATIM
)

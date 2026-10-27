# MiniExchange

Однопоточная библиотека C++17 для сопоставления лимитных заявок одного инструмента.
Заявки исполняются по приоритету цены и FIFO, с поддержкой частичного исполнения.

## API

- @ref mini_exchange::OrderBook "OrderBook" — добавление заявок и снимки стакана.
- @ref mini_exchange::Order "Order" — лимитная заявка: ID, сторона, цена и количество.
- @ref mini_exchange::Trade "Trade" — результат одного исполнения.
- @ref mini_exchange::Side "Side" — покупка или продажа.

Цены задаются в целых тиках, количество — в целых единицах.
Библиотека не выполняет ввод/вывод; пример использования находится в `app/main.cpp`.

[Репозиторий и инструкции по сборке](https://github.com/TEMPORARY1953/MiniExchange)
· [Алгоритм и пример](https://github.com/TEMPORARY1953/MiniExchange/blob/main/Pipeline.md)

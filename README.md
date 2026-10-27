# MiniExchange

Учебный однопоточный Matching Engine на C++17 для одного инструмента.
Ядро оформлено как библиотека без ввода/вывода; консольный пример показывает её работу.

[GitHub Actions](https://github.com/TEMPORARY1953/MiniExchange/actions)
· [Документация API](https://temporary1953.github.io/MiniExchange/)

## Возможности

- Лимитные заявки `BUY` и `SELL`, приоритет цены и FIFO внутри уровня.
- Полное и частичное исполнение по цене заявки, уже стоящей в стакане.
- Сохранение неисполненного остатка и получение снимка стакана.
- Проверка входных данных и уникальности ID за всё время жизни стакана.

Цена задаётся в целых тиках, количество — в целых единицах.
Описание алгоритма и пример: [Pipeline.md](Pipeline.md).

## Структура

| Путь | Назначение |
| --- | --- |
| `include/mini_exchange/` | Публичный API |
| `src/` | Ядро Matching Engine |
| `app/` | Консольный пример |
| `tests/` | Unit-тесты GoogleTest |
| `benchmarks/` | Замеры Google Benchmark |
| `docs/`, `Doxyfile` | Главная страница и настройки Doxygen |
| `.github/workflows/ci.yml` | Сборка, тесты, бенчмарки и публикация документации |

## Сборка и запуск

Нужны CMake 3.24+ и компилятор с поддержкой C++17.
При первой конфигурации CMake загружает GoogleTest 1.17.0 и Google Benchmark 1.9.4 с GitHub.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
cmake --build build --config Release --target run_demo
```

Для Visual Studio 2022 в первой команде можно указать `-G "Visual Studio 17 2022" -A x64`.

## Тесты и бенчмарки

```sh
ctest --test-dir build -C Release --output-on-failure --no-tests=error
cmake --build build --config Release --target run_benchmarks
```

Результаты бенчмарков выводятся в консоль и сохраняются в `build/benchmark.json`.
Методика и результаты: [test&research.md](test&research.md).

## Документация

Для локальной генерации нужен Doxygen 1.9.1+:

```sh
cmake -S . -B build-docs -DBUILD_TESTING=OFF -DMINI_EXCHANGE_BUILD_BENCHMARKS=OFF -DMINI_EXCHANGE_BUILD_DOCS=ON
cmake --build build-docs --target docs
```

Откройте `build-docs/docs/html/index.html`.
GitHub Actions при каждом push выполняет сборку, тесты, бенчмарки и генерацию Doxygen;
из основной ветки документация публикуется на GitHub Pages.

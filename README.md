# Mini Exchange

Учебный движок сопоставления лимитных биржевых заявок на **C++17** для первого ДЗ
курса «Программная инженерия и C++ для количественного анализа и алгоритмической торговли».
Один экземпляр стакана обслуживает один инструмент. Основная логика оформлена
как независимая библиотека; консольное приложение демонстрирует её работу.

## Возможности и правила

- Добавление лимитных заявок `BUY` и `SELL`.
- Приоритет цены: сначала самая дешёвая продажа или самая дорогая покупка.
- При равной цене — FIFO по порядку вызовов `add_order()`, независимо от ID.
- Полное и частичное исполнение, проход через несколько ценовых уровней.
- Сделка по цене **уже стоящей в стакане** заявки. Неисполненный остаток
  входящей заявки сохраняется по её собственной лимитной цене.
- Возврат сделок и копий оставшихся заявок без консольного ввода/вывода в библиотеке.
- Проверка входных данных: `id > 0`, `price > 0`, `quantity > 0`, допустимая сторона.
  Повторный ID запрещён в течение всей жизни стакана, в том числе после исполнения.
  Ошибка проверки вызывает `std::invalid_argument` и не меняет стакан.

Цены — целые **тики**, количество — целые единицы (`std::int64_t`), ID —
`std::uint64_t`. Например, при тике 0,01 цена `100` означает 1,00 денежной единицы.
Размер тика задаётся соглашением вызывающей стороны; денежных вычислений с `double` нет.

Реализован последовательный однопоточный стакан. В проект не входят отмена и изменение
заявок, рыночные заявки, сеть, HTTP API, веб-интерфейс, реальные биржи и хранение данных.

## Структура

```text
MiniExchange/
├── .github/workflows/ci.yml     # сборка, тесты, бенчмарк, Doxygen и Pages
├── include/mini_exchange/
│   └── order_book.hpp          # публичные типы и интерфейс библиотеки
├── src/
│   ├── CMakeLists.txt
│   └── order_book.cpp          # matching и хранение стакана
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp                # консольная демонстрация
├── tests/
│   ├── CMakeLists.txt
│   └── order_book_test.cpp     # GoogleTest
├── benchmarks/
│   ├── CMakeLists.txt
│   └── order_book_benchmark.cpp # Google Benchmark
├── CMakeLists.txt
├── Doxyfile
├── README.md
├── Pipeline.md                 # последовательность обработки заявки
├── test&research.md            # методика и фактические результаты проверок
├── AGENTS.md                   # правила работы Codex
├── .gitignore
└── hw.pdf                      # исходное задание
```

## Окружение

- CMake **3.24+**.
- Компилятор C++17: GCC, Clang или MSVC (Visual Studio 2022 с компонентом Desktop development with C++).
- Make, Ninja или MSBuild — в зависимости от генератора CMake.
- Доступ к GitHub при первой конфигурации: CMake автоматически скачивает
  **GoogleTest 1.17.0** и **Google Benchmark 1.9.4**. Версии и SHA-256 архивов закреплены.
- Для документации — Doxygen **1.9.1+**. Graphviz, LaTeX и Python не нужны.

В Ubuntu/Debian средства сборки и документации можно установить командой:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake doxygen
```

Версия CMake из пакетов должна быть не ниже 3.24. Команды ниже выполняются из корня
проекта. `--config Release` поддерживает Visual Studio и не мешает Make/Ninja.

## Сборка и запуск

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
cmake --build build --config Release --target run_demo
```

В PowerShell с установленным Visual Studio 2022 можно явно выбрать генератор:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 2
cmake --build build --config Release --target run_demo
```

При смене генератора используйте новую директорию сборки.
Для запуска напрямую: `./build/app/mini_exchange_demo` (Make/Ninja)
или `.\build\app\Release\mini_exchange_demo.exe` (Visual Studio).

Пример вывода:

```text
Mini Exchange: prices in ticks, quantities in units
ADD #1 SELL price=100 quantity=5
ADD #2 SELL price=100 quantity=4
ADD #3 SELL price=101 quantity=6
ADD #4 BUY price=101 quantity=12
  TRADE buy=#4 sell=#1 price=100 quantity=5
  TRADE buy=#4 sell=#2 price=100 quantity=4
  TRADE buy=#4 sell=#3 price=101 quantity=3
ADD #5 BUY price=99 quantity=7
ADD #6 SELL price=98 quantity=2
  TRADE buy=#5 sell=#6 price=99 quantity=2
Remaining orders (best price, then FIFO):
BUY:
  #5 price=99 quantity=5
SELL:
  #3 price=101 quantity=3
```

## Тесты и бенчмарк

```bash
ctest --test-dir build -C Release --output-on-failure --no-tests=error
cmake --build build --config Release --target run_benchmarks
```

Цель `run_benchmarks` выполняет шесть замеров: добавление без сделок, встречные
пары и проход через 32 ценовых уровня, каждый при размерах 128 и 4096.
Используются три повторения и минимальное время 0,1 с на замер.
`Time` и `CPU` относятся ко **всей итерации (пакету)**, `items_per_second` — к
числу входящих заявок в секунду. Результаты выводятся в консоль и журнал CI,
а также сохраняются в `build/benchmark.json`.

Для сохранения JSON можно запустить исполняемый файл напрямую:

```bash
./build/benchmarks/order_book_benchmark --benchmark_min_time=0.1s --benchmark_repetitions=3 --benchmark_out=build/benchmark.json --benchmark_out_format=json
```

Для Visual Studio путь: `.\build\benchmarks\Release\order_book_benchmark.exe`.
Методика, состав тестов, результаты и ограничения измерений описаны в
[test&research.md](test&research.md).

Локально проверено на MSVC 19.44 в Release: **22/22 теста**, консольный пример,
все шесть бенчмарков и генерация Doxygen 1.13.2 без предупреждений.
Подробные измерения и ограничения приведены в отчёте; удалённый CI пока не проверен.

Для сборки только библиотеки и примера, без загрузки зависимостей:

```bash
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DMINI_EXCHANGE_BUILD_BENCHMARKS=OFF
cmake --build build-core --config Release --parallel 2
```

## Архитектура и использование библиотеки

`Order`, `Trade`, `Side` и `OrderBook` находятся в пространстве имён `mini_exchange`.
`OrderBook::add_order()` принимает значение `Order` и возвращает `std::vector<Trade>`.
`orders(Side)` возвращает независимый снимок соответствующей стороны стакана,
упорядоченный по цене и времени. История сделок внутри стакана не хранится.

```cpp
#include <mini_exchange/order_book.hpp>

mini_exchange::OrderBook book;
auto first = book.add_order({1, mini_exchange::Side::Sell, 100, 5}); // нет сделок
auto trades = book.add_order({2, mini_exchange::Side::Buy, 101, 3}); // 3 по цене 100
auto asks = book.orders(mini_exchange::Side::Sell); // остаток #1: 2 единицы
```

Цель CMake для подключения: `MiniExchange::mini_exchange`.
Для будущего интерфейса достаточно вызывать эти методы из отдельного адаптера
и преобразовывать возвращённые структуры в нужный формат. Ядро не зависит от UI.
При доступе из нескольких потоков вызывающий код должен обеспечить синхронизацию.

Внутри используются две `std::map` ценовых уровней и `std::deque<Order>` на уровне.
Покупки отсортированы по убыванию цены, продажи — по возрастанию. `unordered_set`
помнит использованные ID. Добавление уровня стоит `O(log L)`, где `L` — число
уровней; исполнение одной заявки зависит от количества затронутых заявок и уровней.
Снимок требует `O(N)` времени и памяти для `N` заявок выбранной стороны.
Память для ID растёт с числом принятых заявок за жизнь стакана — это осознанное
упрощение учебного проекта. Восстановление после нехватки памяти не предусмотрено.

Подробная последовательность и пример частичного исполнения: [Pipeline.md](Pipeline.md).

## Doxygen и GitHub Pages

**Планируемый адрес:** [документация Mini Exchange](https://temporary1953.github.io/MiniExchange/?utm_source=chatgpt.com).
Это адрес для репозитория `TEMPORARY1953/MiniExchange`;

Локальная генерация, включая проверку предупреждений Doxygen:

```bash
cmake -S . -B build-docs -DBUILD_TESTING=OFF -DMINI_EXCHANGE_BUILD_BENCHMARKS=OFF -DMINI_EXCHANGE_BUILD_DOCS=ON
cmake --build build-docs --target docs
```

Откройте `build-docs/docs/html/index.html`. Цель CMake сама задаёт переменную
`MINI_EXCHANGE_DOCS_DIR`, используемую в `Doxyfile`. При непосредственном запуске
`doxygen Doxyfile` эту переменную нужно задать вручную.

## Автоматические проверки

Workflow [.github/workflows/ci.yml](.github/workflows/ci.yml) запускается на **каждый push**,
на pull request и вручную через `workflow_dispatch`.

1. На Ubuntu 24.04 и Windows Server 2022 собирает Release, выполняет GoogleTest
   через CTest, демонстрацию и Google Benchmark. Замеры видны в шаге
   `Measure order processing`; JUnit-отчёты и JSON бенчмарка доступны в артефактах `tests-*`.
2. На Ubuntu генерирует Doxygen. Предупреждения считаются ошибками.
   HTML всегда доступен как артефакт `doxygen-html`, включая pull request.
3. После успешных проверок публикует HTML на Pages при push в **основную ветку**
   или ручном запуске из неё. Ветка определяется из настроек репозитория.
   Pull request и другие ветки проверяются без публикации.

Публикация использует официальные `configure-pages`, `upload-pages-artifact`,
`deploy-pages`, окружение `github-pages` и права `pages: write`, `id-token: write`.
Персональный токен в secrets не требуется. Настройки соответствуют
[инструкции GitHub Pages](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages).

## Первая публикация и проверка ДЗ

На момент локальной реализации исходная папка не содержала `.git` и remote.
Успешный запуск GitHub Actions и доступность Pages пока не проверены.
Для полных баллов за пункты 2–5 нужны успешные проверки **на GitHub**, как указано в `hw.pdf`.

1. Создайте на GitHub **публичный** пустой репозиторий `mini-exchange`
   (без автоматически создаваемых README и `.gitignore`).
2. Из корня проекта выполните, заменив `OWNER` своим логином:

   ```bash
   git init -b main
   git add .
   git commit -m "Implement Mini Exchange with tests, benchmarks and documentation"
   git remote add origin https://github.com/OWNER/mini-exchange.git
   git push -u origin main
   ```

   `.tools/`, `build*/` и локальные зависимости исключены через `.gitignore`.
   При необходимости выполните `gh auth login -h github.com` для авторизации CLI.
3. В репозитории откройте **Settings → Pages → Build and deployment → Source → GitHub Actions**.
   В **Settings → Actions → General** убедитесь, что GitHub Actions разрешены.
   При наличии правил окружения `github-pages` разрешите деплой из основной ветки.
4. Откройте **Actions → Build, test, benchmark and documentation → Run workflow**, выберите `main`.
   Повторный запуск нужен, если первый push произошёл до включения Pages.
5. Проверьте зелёный статус обеих `Verify`, `Build Doxygen documentation` и `Publish GitHub Pages`.
   Откройте журналы тестов и бенчмарка; перейдите по URL окружения `github-pages`
   и проверьте главную страницу и описание `OrderBook`.
6. Уточните ссылку на Pages выше и добавьте ссылку на успешный run в
   `test&research.md`. Закоммитьте изменения: следующий push повторит проверки.

## Соответствие домашнему заданию

| Пункт | Баллы | Реализация | Что нужно подтвердить на GitHub |
| --- | ---: | --- | --- |
| Репозиторий | 2 | Раздельные `src`, `tests`, `benchmarks`, README | Публичность репозитория |
| CMake и CI/CD | 4 | CMake + Actions, Linux и Windows | Успешная сборка на push |
| Unit-тесты | 4 | GoogleTest + CTest | Успешный шаг тестов |
| Бенчмарк | 4 | Google Benchmark, 6 замеров | Численные результаты в журнале |
| Документация | 4 | Doxygen + Pages, ссылка выше | Успешный deploy и открывающийся сайт |
| AGENTS.md | 2 | Назначение, структура, команды, стиль, ограничения | Файл в корне репозитория |

Проект подготовлен для всех шести пунктов; получение 20/20 до удалённых проверок
не заявляется. Фактические локальные результаты приведены в [отчёте](test&research.md).

Организация документации основана на изучении
[референса Monte-Carlo-GPU/Project](https://github.com/Monte-Carlo-GPU/Project):
README описывает назначение, окружение, архитектуру и запуск, `Pipeline.md` —
обработку данных, `test&research.md` — выполненные проверки. Код, CUDA-зависимости
и предметная логика референса не используются.

# Отчёт №6

## Цель работы

- Реализовать условную компиляцию с макросами `USE_GPU` и `USE_FLOAT` и проверить её работу через препроцессор.
- Сравнить производительность программы `sha1-benchmark` при разных уровнях оптимизации компилятора.
- Оценить влияние оптимизации линковки (LTO) и оптимизации на основе профиля выполнения (PGO).

## Среда выполнения

- ОС: Ubuntu
- Компилятор: `g++`

## Файлы 6 параграфа

- `01_conditional_compilation_demo.c`
- `sha1.cc`
- `sha1_benchmark.cc`
- `report_paragraph_6_corrected.md`

> Файл `a.out` не следует хранить как основной файл параграфа, так как это результат компиляции. Его лучше удалять после проверки или исключать из репозитория.

---

## 1) Условная компиляция (проверка через `g++ -E`)

### Файл

`01_conditional_compilation_demo.c`

### Код

```c
#include <stdio.h>

static void process(void)
{
#if defined(USE_GPU) && defined(USE_FLOAT)
    printf("USE_GPU and USE_FLOAT\n");
#elif defined(USE_GPU)
    printf("USE_GPU\n");
#elif defined(USE_FLOAT)
    printf("USE_FLOAT\n");
#else
    printf("None\n");
#endif
}

int main(void)
{
    process();
    return 0;
}
```

### Проверка препроцессора

```bash
g++ -E 01_conditional_compilation_demo.c | grep 'None'
g++ -E -DUSE_GPU 01_conditional_compilation_demo.c | grep 'USE_'
g++ -E -DUSE_FLOAT 01_conditional_compilation_demo.c | grep 'USE_'
g++ -E -DUSE_GPU -DUSE_FLOAT 01_conditional_compilation_demo.c | grep 'USE_'
```

### Результат

```text
printf("None\n");
printf("USE_GPU\n");
printf("USE_FLOAT\n");
printf("USE_GPU and USE_FLOAT\n");
```

### Пояснение

- Директивы `#if`, `#elif` и `#else` задают четыре возможные ветви компиляции в зависимости от наличия макросов.
- Ключ `-D` при вызове `g++` определяет макрос прямо из командной строки без изменения исходного файла.
- Команда `g++ -E` запускает только препроцессор и показывает текст программы после обработки директив.
- Использование `grep` позволяет быстро выделить строку `printf`, соответствующую выбранной ветви.
- Выделение логики в функцию `process()` делает пример чище и нагляднее, чем размещение всех директив внутри `main()`.

### Пример запуска

```bash
cd "Paragraph 6"
g++ -E 01_conditional_compilation_demo.c | grep 'None'
g++ -E -DUSE_GPU 01_conditional_compilation_demo.c | grep 'USE_'
g++ -E -DUSE_FLOAT 01_conditional_compilation_demo.c | grep 'USE_'
g++ -E -DUSE_GPU -DUSE_FLOAT 01_conditional_compilation_demo.c | grep 'USE_'
```

---

## 2) Тесты производительности (`-O0` и `-O3`)

### Сборка

```bash
g++ -std=c++17 -O0 -o sha1-bench-O0 sha1_benchmark.cc sha1.cc
g++ -std=c++17 -O3 -march=native -o sha1-bench-O3 sha1_benchmark.cc sha1.cc
```

### Измерение времени

```bash
for i in {1..5}; do /usr/bin/time -f "O0 real=%e" sh -c './sha1-bench-O0 >/dev/null 2>&1'; done
for i in {1..5}; do /usr/bin/time -f "O3 real=%e" sh -c './sha1-bench-O3 >/dev/null 2>&1'; done
```

### Результаты

- O0: 20.90, 17.41, 18.29, 18.02, 18.37 с
- среднее время O0: **18.598 с**

- O3: 5.04, 4.92, 4.89, 4.89, 4.85 с
- среднее время O3: **4.918 с**

Итог:

- ускорение: **3.78×**
- уменьшение времени: **73.6%**

### Пояснение

- Опция `-O0` отключает оптимизацию и используется как базовый вариант для сравнения.
- Опция `-O3` включает агрессивные оптимизации компилятора.
- Ключ `-march=native` разрешает использовать возможности процессора текущей машины.
- Команда `/usr/bin/time` измеряет реальное время выполнения программы.
- Перенаправление `>/dev/null 2>&1` убирает вывод программы и позволяет измерять именно время работы без лишнего текста в терминале.

### Пример запуска

```bash
cd "Paragraph 6"
g++ -std=c++17 -O0 -o sha1-bench-O0 sha1_benchmark.cc sha1.cc
g++ -std=c++17 -O3 -march=native -o sha1-bench-O3 sha1_benchmark.cc sha1.cc
for i in {1..5}; do /usr/bin/time -f "O0 real=%e" sh -c './sha1-bench-O0 >/dev/null 2>&1'; done
for i in {1..5}; do /usr/bin/time -f "O3 real=%e" sh -c './sha1-bench-O3 >/dev/null 2>&1'; done
```

---

## 3) Оптимизация во время линковки (LTO)

### Сборка

```bash
g++ -std=c++17 -O0 -flto -o sha1-bench-O0-lto sha1_benchmark.cc sha1.cc
g++ -std=c++17 -O3 -march=native -flto -o sha1-bench-O3-lto sha1_benchmark.cc sha1.cc
```

### Измерение времени

```bash
for i in {1..5}; do /usr/bin/time -f "O0+LTO real=%e" sh -c './sha1-bench-O0-lto >/dev/null 2>&1'; done
for i in {1..5}; do /usr/bin/time -f "O3+LTO real=%e" sh -c './sha1-bench-O3-lto >/dev/null 2>&1'; done
```

### Результаты

- O0+LTO: 20.88, 17.42, 17.39, 17.44, 17.49 с
- среднее время O0+LTO: **18.124 с**

- O3+LTO: 4.57, 4.62, 4.59, 4.59, 4.57 с
- среднее время O3+LTO: **4.588 с**

Итог для O3:

- ускорение O3+LTO относительно O3: **1.07×**
- уменьшение времени: **6.7%**

### Пояснение

- Ключ `-flto` включает оптимизацию на этапе линковки.
- В этом режиме компилятор получает больше информации о программе целиком и может дополнительно упростить код между модулями.
- Для варианта без оптимизации эффект оказался небольшим, тогда как для `O3` наблюдается дополнительное снижение времени выполнения.
- Сравнение проводится с уже оптимизированной версией `O3`, поэтому прирост здесь заметно меньше, чем при переходе от `O0` к `O3`.

### Пример запуска

```bash
cd "Paragraph 6"
g++ -std=c++17 -O0 -flto -o sha1-bench-O0-lto sha1_benchmark.cc sha1.cc
g++ -std=c++17 -O3 -march=native -flto -o sha1-bench-O3-lto sha1_benchmark.cc sha1.cc
for i in {1..5}; do /usr/bin/time -f "O0+LTO real=%e" sh -c './sha1-bench-O0-lto >/dev/null 2>&1'; done
for i in {1..5}; do /usr/bin/time -f "O3+LTO real=%e" sh -c './sha1-bench-O3-lto >/dev/null 2>&1'; done
```

---

## 4) Оптимизация с помощью профилирования (PGO)

### Сборка с генерацией профиля и прогон

```bash
rm -rf pgo && mkdir pgo
g++ -std=c++17 -O3 -march=native -flto -fprofile-generate -fprofile-dir=pgo -o sha1-bench-pgo-gen sha1_benchmark.cc sha1.cc
for i in {1..5}; do ./sha1-bench-pgo-gen >/dev/null 2>&1; done
```

### Пересборка с использованием профиля

```bash
g++ -std=c++17 -O3 -march=native -flto -fprofile-use -fprofile-dir=pgo -fprofile-correction -Wno-missing-profile -o sha1-bench-O3-lto-pgo sha1_benchmark.cc sha1.cc
```

### Измерение времени

```bash
for i in {1..5}; do /usr/bin/time -f "baseline O3+LTO real=%e" sh -c './sha1-bench-O3-lto >/dev/null 2>&1'; done
for i in {1..5}; do /usr/bin/time -f "PGO O3+LTO real=%e" sh -c './sha1-bench-O3-lto-pgo >/dev/null 2>&1'; done
```

### Результаты

- baseline O3+LTO: 4.55, 4.55, 4.54, 4.54, 4.74 с
- среднее время baseline: **4.584 с**

- PGO O3+LTO: 4.74, 4.81, 4.73, 4.76, 4.74 с
- среднее время PGO: **4.756 с**

Итог:

- замедление PGO относительно baseline: **1.04×**
- увеличение времени: **3.7%**

### Пояснение

- На первом этапе `-fprofile-generate` добавляет в программу сбор статистики выполнения.
- После нескольких запусков формируется профиль, который затем используется при пересборке с ключом `-fprofile-use`.
- Параметр `-fprofile-dir=pgo` задаёт каталог для хранения профиля.
- В данном случае использование PGO не ускорило программу, а немного увеличило среднее время выполнения.
- Это показывает, что профилирование полезно не во всех сценариях и его результат зависит от характера входных данных и качества собранного профиля.

### Пример запуска

```bash
cd "Paragraph 6"
rm -rf pgo && mkdir pgo
g++ -std=c++17 -O3 -march=native -flto -fprofile-generate -fprofile-dir=pgo -o sha1-bench-pgo-gen sha1_benchmark.cc sha1.cc
for i in {1..5}; do ./sha1-bench-pgo-gen >/dev/null 2>&1; done
g++ -std=c++17 -O3 -march=native -flto -fprofile-use -fprofile-dir=pgo -fprofile-correction -Wno-missing-profile -o sha1-bench-O3-lto-pgo sha1_benchmark.cc sha1.cc
for i in {1..5}; do /usr/bin/time -f "PGO O3+LTO real=%e" sh -c './sha1-bench-O3-lto-pgo >/dev/null 2>&1'; done
```

---

## Вывод

В 6 параграфе были рассмотрены четыре приёма оптимизации и проверки кода. В файле `01_conditional_compilation_demo.c` условная компиляция была проверена через `g++ -E` и макросы `-DUSE_GPU`, `-DUSE_FLOAT`, при этом каждая комбинация выбирала свою ветвь препроцессора. По измерениям `sha1-benchmark` переход от `-O0` к `-O3 -march=native` сократил среднее время с **18.598 с** до **4.918 с**, а добавление `-flto` уменьшило его ещё до **4.588 с**. Вариант с PGO (`-fprofile-generate` и `-fprofile-use`) в этих запусках оказался хуже базового `O3+LTO` и дал среднее время **4.756 с**. Для структуры папки целесообразно хранить исходный файл под содержательным именем, а не использовать общее имя `main.c`, и не оставлять в ней артефакт компиляции `a.out` как основной файл параграфа.

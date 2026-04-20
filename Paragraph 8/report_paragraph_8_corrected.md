# Отчёт №8

## Цель работы

1. Описать зависимости сборки в Make так, чтобы пересборка происходила при изменении заголовочного файла.
2. Подключить библиотеку **zlib** в Make через `pkg-config`.
3. Собрать код с библиотекой **zlib** с помощью Meson.
4. Склонировать проект `unistdx` и собрать его Meson с оптимизациями.

## Файлы 8 параграфа

- `zlib_build_demo.cc`
- `zlib_build_demo.hh`
- `Makefile`
- `meson.build`
- `report_paragraph_8_corrected.md`

## Исходные файлы

### `zlib_build_demo.cc`

```cpp
#include <zlib.h>
#include "zlib_build_demo.hh"

const char* get_zlib_version()
{
    return zlibVersion();
}

int main()
{
    return get_zlib_version()[0] == '\0';
}
```

### `zlib_build_demo.hh`

```cpp
#pragma once

const char* get_zlib_version();
```

---

## 1) Make: зависимости для пересборки при изменении заголовка

### Решение

```make
all: zlib_build_demo

zlib_build_demo: zlib_build_demo.o

zlib_build_demo.o: zlib_build_demo.cc zlib_build_demo.hh
```

### Пояснение

- Цель `all` делает основной целью сборки исполняемый файл `zlib_build_demo`.
- Правило `zlib_build_demo: zlib_build_demo.o` показывает, что исполняемый файл зависит от объектного файла.
- Правило `zlib_build_demo.o: zlib_build_demo.cc zlib_build_demo.hh` явно указывает зависимость объектного файла и от исходного файла, и от заголовка.
- Благодаря этому при изменении `zlib_build_demo.hh` Make будет считать `zlib_build_demo.o` устаревшим и выполнит его пересборку.

---

## 2) Make + pkg-config: сборка с библиотекой zlib

### Файл `Makefile`

```make
CXX := g++
TARGET := zlib_build_demo
SRC := zlib_build_demo.cc
OBJ := zlib_build_demo.o

CPPFLAGS := $(shell pkg-config --cflags zlib)
LDLIBS := $(shell pkg-config --libs zlib)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDLIBS)

$(OBJ): $(SRC) zlib_build_demo.hh
	$(CXX) $(CPPFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)
```

### Пояснение

- `pkg-config --cflags zlib` добавляет флаги компиляции, необходимые для подключения заголовков библиотеки.
- `pkg-config --libs zlib` добавляет флаги линковки.
- Переменная `TARGET` названа так же, как и исходный файл задачи, что делает структуру проекта более понятной.
- Зависимость `$(OBJ): $(SRC) zlib_build_demo.hh` сохраняет корректную пересборку при изменении заголовочного файла.
- Цель `clean` удаляет промежуточный объектный файл и готовый исполняемый файл.

### Команды проверки

```bash
make
make clean
```

---

## 3) Meson: сборка кода с zlib

### Файл `meson.build`

```meson
project('zlib_build_demo', 'cpp', default_options : ['cpp_std=c++17'])

zlib_dep = dependency('zlib', required : true)

executable('zlib_build_demo', 'zlib_build_demo.cc', dependencies : zlib_dep)
```

### Команды сборки

```bash
meson setup builddir
meson compile -C builddir
```

### Пояснение

- Команда `project(...)` задаёт имя проекта и стандарт языка C++.
- Функция `dependency('zlib', required : true)` поручает Meson найти установленную библиотеку `zlib` и завершить сборку с ошибкой, если она недоступна.
- Команда `executable(...)` описывает исполняемый файл `zlib_build_demo` и связывает его с найденной зависимостью.
- `meson setup builddir` подготавливает отдельный каталог сборки.
- `meson compile -C builddir` выполняет сборку в уже настроенной директории.

---

## 4) Git + Meson: сборка `unistdx` с оптимизациями

### Использованные команды

```bash
git clone https://github.com/igankevich/unistdx.git
cd unistdx
meson setup builddir -Dc_args="-march=native -O3" -Dcpp_link_args="-flto"
meson compile -C builddir
```

### Пояснение

- Перед выполнением сборки проект `unistdx` предварительно клонируется через `git clone`.
- Флаг `-Dc_args="-march=native -O3"` передаёт компилятору параметры оптимизации: использование возможностей текущего процессора и высокий уровень оптимизации.
- Параметр `-Dcpp_link_args="-flto"` добавляет оптимизацию на этапе линковки.
- После настройки директории `builddir` команда `meson compile -C builddir` запускает сборку проекта с указанными параметрами.

---

## Вывод

В 8 параграфе были использованы два способа описания сборки: через Make и через Meson. В `Makefile` была задана зависимость объектного файла от исходного файла и заголовка, а подключение `zlib` выполнено через `pkg-config --cflags` и `pkg-config --libs`. В `meson.build` та же библиотека была подключена через `dependency('zlib', required : true)`, а исполняемый файл описан как `zlib_build_demo`. На последнем этапе был показан пример сборки стороннего проекта `unistdx` с оптимизациями `-march=native`, `-O3` и `-flto`.

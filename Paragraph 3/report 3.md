# Отчёт №3

## Цель работы

- Закрепить навыки написания shell-скриптов, включая проверку аргументов, использование условного оператора `case` и разбор опций командной строки через `getopts`.
- Освоить построение функции-обёртки для запуска команды с перенаправлением стандартного вывода и потока ошибок в файл.
- Научиться создавать скрипт-генератор, формирующий команды переименования файлов с переводом имён в верхний регистр.

## Среда выполнения

- ОС: Ubuntu
- Используемые утилиты: `file`, `curl`, `tr`, `mv`, `gimp`, `libreoffice`, `mpv`

## Файлы 3 параграфа

- `01_open_file_by_mime_type.sh`
- `02_get_weather_by_coordinates.sh`
- `03_run_command_with_output_redirect.sh`
- `04_generate_uppercase_rename_script.sh`
- `05_rename_files_to_uppercase.sh`
- `test_open_file_by_mime_type/`
- `test_generate_uppercase_rename_script/`

---

## Ход работы

## 1) Открытие файлов по MIME-типу

### Файл

`01_open_file_by_mime_type.sh`

### Задача

Написать скрипт, который определяет MIME-тип файла и открывает его подходящей программой. Для определения типа используется `file` с флагом вывода MIME-типа.

### Решение

```bash
#!/bin/bash
if [ $# -ne 1 ] || [ ! -f "$1" ]; then
    echo "usage: open [file]"
    echo "Ensure that only one existing file is transferred"
    exit 1
fi

FILE="$1"
MIME_TYPE=$(file --mime-type -b "$FILE")

case "$MIME_TYPE" in
    image/*)
        nohup gimp "$FILE" &>/dev/null &
        ;;
    application/msword | application/vnd.ms-excel | application/vnd.ms-powerpoint | application/vnd.openxmlformats-officedocument.wordprocessingml.document)
        nohup libreoffice "$FILE" &>/dev/null &
        ;;
    video/* | audio/*)
        nohup mpv "$FILE" &>/dev/null &
        ;;
    *)
        echo "Unknown MIME type: $MIME_TYPE.
No suitable program to open the file"
        exit 1
        ;;
esac

exit 0
```

### Пояснение

- Проверка `if [ $# -ne 1 ] || [ ! -f "$1" ]` используется для контроля входных данных: скрипт должен получить ровно один аргумент, причём этот аргумент обязан ссылаться на существующий файл.
- Команда `file --mime-type -b "$FILE"` определяет MIME-тип файла без вывода имени файла, что позволяет использовать только сам тип в дальнейшей логике.
- Оператор `case` организует выбор программы в зависимости от полученного MIME-типа.
- Конструкция `nohup ... &` запускает приложение в фоновом режиме и позволяет ему продолжить работу после закрытия текущего терминала.
- Перенаправление `&>/dev/null` подавляет стандартный вывод и сообщения об ошибках, чтобы запуск графической программы не сопровождался лишним текстом в консоли.
- В текущем варианте скрипт поддерживает три группы MIME-типов: изображения, часть офисных документов и аудио/видео.

### Пример запуска

```bash
./01_open_file_by_mime_type.sh test_open_file_by_mime_type/1.jpg
```

---

## 2) Аргументы: погода по координатам (`getopts`)

### Файл

`02_get_weather_by_coordinates.sh`

### Задача

Написать скрипт, принимающий опции `-x` и `-y`, и выводящий погоду в точке с долготой `X` и широтой `Y`, используя сервис `wttr.in`. Для разбора опций используется `getopts`.

### Решение

```bash
#!/bin/bash
LATITUDE=""
LONGITUDE=""

while getopts "x:y:" option; do
    case "$option" in
        x)
            LONGITUDE="$OPTARG"
            ;;
        y)
            LATITUDE="$OPTARG"
            ;;
        *)
            echo "Usage: $0 -x  -y "
            exit 1
            ;;
    esac
done

if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
    echo "Usage: $0 -x  -y "
    exit 1
fi

echo "Getting weather for coordinates: Latitude=$LATITUDE, Longitude=$LONGITUDE"
curl "wttr.in/$LATITUDE,$LONGITUDE"
```

### Пояснение

- Механизм `getopts` предназначен для последовательного разбора опций командной строки в shell-скриптах.
- Строка `"x:y:"` означает, что и у `-x`, и у `-y` должно быть собственное значение.
- В данном скрипте опция `-x` записывается в переменную `LONGITUDE`, а `-y` — в `LATITUDE`, после чего запрос формируется как `wttr.in/широта,долгота`.
- Переменная `OPTARG` содержит значение текущей опции и используется для записи переданных координат.
- Проверка с помощью `-z` позволяет удостовериться, что обе координаты заданы до выполнения запроса.
- Команда `curl` обращается к сервису `wttr.in`, формируя запрос на основе полученных координат.
- Сообщение `Usage` в текущем коде оставлено в краткой форме и не содержит подписей параметров; это соответствует фактическому содержимому скрипта.

### Пример запуска

```bash
./02_get_weather_by_coordinates.sh -x 30 -y 59
```

---

## 3) Функции: запуск команды с перенаправлением `stdout`/`stderr` в файл

### Файл

`03_run_command_with_output_redirect.sh`

### Задача

Написать функцию, которая принимает первым аргументом имя файла, а далее — команду с аргументами, и перенаправляет стандартный вывод и ошибки в указанный файл.

### Решение

```bash
#!/bin/bash

my_func() {
    if [ $# -lt 2 ]; then
        echo "Usage: my_func  [args...]"
        return 1
    fi

    OUTPUT_FILE="$1"
    shift  # Убираем первый аргумент, чтобы остались только команда и её аргументы
    "$@" >"$OUTPUT_FILE" 2>&1
}

my_func ./my-output ls -l
```

### Пояснение

- Функция `my_func` реализует универсальную обёртку для запуска произвольной команды с перенаправлением результата в файл.
- Условие `[ $# -lt 2 ]` проверяет, что пользователем переданы как минимум имя выходного файла и сама команда.
- Команда `shift` удаляет первый аргумент из списка параметров функции, вследствие чего в `$@` остаются только имя команды и её аргументы.
- Перенаправление `>"$OUTPUT_FILE"` записывает стандартный вывод в указанный файл.
- Конструкция `2>&1` направляет поток ошибок туда же, куда уже перенаправлен стандартный вывод.
- Оператор `return` используется именно потому, что завершение происходит внутри функции, а не на уровне всего скрипта.
- В демонстрационном вызове результат команды `ls -l` записывается в файл `my-output`.

### Пример запуска

```bash
./03_run_command_with_output_redirect.sh
cat my-output
```

---

## 4) Генерация файлов: создание скрипта переименования в верхний регистр

### Файлы

- `04_generate_uppercase_rename_script.sh`
- `05_rename_files_to_uppercase.sh`

### Задача

Написать скрипт, который генерирует другой скрипт, переименовывающий все файлы в директории, меняя регистр имени на верхний. Для изменения регистра используется `tr`, для переименования — `mv`.

### Решение

```bash
#!/bin/bash
if [ $# -ne 1 ]; then
    echo "Usage: $0 "
    exit 1
fi

DIRECTORY="$1"

if [ ! -d "$DIRECTORY" ]; then
    echo "$DIRECTORY is not a valid directory."
    exit 1
fi

DIRECTORY=$(echo "$DIRECTORY" | sed 's:/*$::')
GENERATED_SCRIPT="05_rename_files_to_uppercase.sh"

echo "#!/bin/bash" >"$GENERATED_SCRIPT"

for FILE in "$DIRECTORY"/*; do
    if [ -f "$FILE" ]; then
        BASENAME=$(basename "$FILE")
        NAME="${BASENAME%.*}"
        EXT="${BASENAME##*.}"
        NEW_NAME=$(echo "$NAME" | tr 'a-z' 'A-Z')
        NEW_FILE="$DIRECTORY/$NEW_NAME.$EXT"

        echo "mv "$FILE" "$NEW_FILE"" >>"$GENERATED_SCRIPT"
    fi
done

chmod +x "$GENERATED_SCRIPT"
echo "$GENERATED_SCRIPT generated."
```

### Пояснение

- Скрипт принимает путь к каталогу и сначала проверяет корректность входного аргумента.
- Команда `sed 's:/*$::'` удаляет завершающие символы `/`, чтобы путь к каталогу имел единообразный вид.
- В переменную `GENERATED_SCRIPT` записывается имя создаваемого файла `05_rename_files_to_uppercase.sh`, которое соответствует текущей структуре папки.
- Цикл `for FILE in "$DIRECTORY"/*` последовательно перебирает элементы указанной директории.
- Для каждого обычного файла выделяются имя без расширения и само расширение, после чего имя преобразуется в верхний регистр с помощью `tr 'a-z' 'A-Z'`.
- Команда `echo ... >>"$GENERATED_SCRIPT"` не выполняет переименование сразу, а записывает готовую строку `mv` в новый скрипт.
- Команда `chmod +x` делает сгенерированный файл исполняемым.
- После запуска `04_generate_uppercase_rename_script.sh` полученный файл `05_rename_files_to_uppercase.sh` можно выполнить отдельно.

### Пример запуска

```bash
./04_generate_uppercase_rename_script.sh test_generate_uppercase_rename_script
cat 05_rename_files_to_uppercase.sh
./05_rename_files_to_uppercase.sh
```

---

## Вывод

В 3 параграфе были реализованы четыре разные задачи на Bash: выбор программы по MIME-типу через `file` и `case`, разбор опций `-x` и `-y` с помощью `getopts`, перенаправление вывода команды в файл через функцию `my_func`, а также генерация отдельного скрипта переименования с использованием `tr`, `mv` и `chmod +x`. Названия файлов приведены к единому формату и согласованы с текущей структурой папки: `01_open_file_by_mime_type.sh`, `02_get_weather_by_coordinates.sh`, `03_run_command_with_output_redirect.sh`, `04_generate_uppercase_rename_script.sh` и `05_rename_files_to_uppercase.sh`.

# Отчёт №10: Ввод/вывод (класс-обёртка File)

## Цель работы

Реализовать класс-обёртку `File` для работы с файловым дескриптором Linux через системные вызовы:

- открытие и закрытие файла (`open`, `close`);
- чтение и запись (`read`, `write`);
- управление текущей позицией в файле (`lseek`);
- поддержку перемещения и копирования файлового дескриптора (`dup`, `dup2`);
- контроль ошибок через функцию `check`.

## Среда выполнения

- ОС: Ubuntu
- Компилятор: `g++`
- Средство дополнительной проверки: `valgrind`

## Файлы 10 параграфа

- `01_linux_file_wrapper.cpp`
- `report_paragraph_10_corrected.md`

> Файл `testfile.txt` создаётся программой во время запуска и не является исходным файлом параграфа. Исполняемый файл `linux_file_wrapper` и возможный `a.out` также относятся к артефактам сборки.

---

## Реализация

Класс `File`:

- в конструкторе открывает файл и сохраняет файловый дескриптор;
- в деструкторе закрывает дескриптор, если он остаётся валидным;
- предоставляет методы `writeToFile()` и `readFromFile()` для записи и чтения данных;
- реализует перемещение ресурса через move-конструктор и move-оператор присваивания;
- поддерживает получение и изменение текущей позиции в файле через `getPosition()` и `setPosition()`;
- реализует копирование через `dup` и `dup2`, то есть создаёт новый дескриптор, ссылающийся на тот же открытый файл;
- использует шаблонную функцию `check`, которая проверяет результат системного вызова и при ошибке генерирует исключение `std::system_error`.

---

## Код программы

### Файл

`01_linux_file_wrapper.cpp`

```cpp
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

template <class T>
inline T check(T ret)
{
    if (ret == T(-1))
        throw std::system_error(errno, std::generic_category());
    return ret;
}

class File
{
private:
    int fd = -1;

    void writeAll(const char *data, size_t size)
    {
        size_t total_written = 0;

        while (total_written < size)
        {
            ssize_t nwritten = check(write(fd, data + total_written, size - total_written));
            total_written += static_cast<size_t>(nwritten);
        }

        std::cout << "Bytes written: " << total_written << std::endl;
    }

public:
    File(const std::string &filename, int flags, mode_t mode = 0644)
    {
        fd = check(open(filename.c_str(), flags, mode));
    }

    ~File() noexcept
    {
        if (fd != -1)
            close(fd);
    }

    File(const File &rhs)
    {
        fd = check(dup(rhs.fd));
    }

    File &operator=(const File &rhs)
    {
        if (this != &rhs)
        {
            if (fd == -1)
                fd = check(dup(rhs.fd));
            else
                fd = check(dup2(rhs.fd, fd));
        }
        return *this;
    }

    File(File &&rhs) noexcept
    {
        fd = rhs.fd;
        rhs.fd = -1;
    }

    File &operator=(File &&rhs) noexcept
    {
        if (this != &rhs)
        {
            if (fd != -1)
                close(fd);

            fd = rhs.fd;
            rhs.fd = -1;
        }
        return *this;
    }

    void writeToFile(const std::string &data)
    {
        writeAll(data.c_str(), data.size());
    }

    std::string readFromFile(size_t size)
    {
        std::vector<char> buffer(size);
        ssize_t nread = check(read(fd, buffer.data(), buffer.size()));
        return std::string(buffer.data(), static_cast<size_t>(nread));
    }

    off_t getPosition()
    {
        return check(lseek(fd, 0, SEEK_CUR));
    }

    void setPosition(off_t offset)
    {
        check(lseek(fd, offset, SEEK_SET));
    }
};

int main()
{
    try
    {
        File file("testfile.txt", O_CREAT | O_WRONLY | O_TRUNC);
        file.writeToFile("Hello, World!\n");

        File file_read("testfile.txt", O_RDONLY);
        std::string content = file_read.readFromFile(256);
        std::cout << "File contents: " << content << std::endl;

        File moved_file = std::move(file_read);
        moved_file.setPosition(0);
        std::string moved_content = moved_file.readFromFile(256);
        std::cout << "Moved file contents: " << moved_content << std::endl;
    }
    catch (const std::system_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// valgrind --track-fds=yes ./linux_file_wrapper
```

---

## Сборка и запуск

```bash
g++ -Wall -Wextra -O2 01_linux_file_wrapper.cpp -o linux_file_wrapper
./linux_file_wrapper
```

В ходе работы программа:

1. создаёт или перезаписывает файл `testfile.txt` и записывает в него строку `Hello, World!`;
2. открывает файл на чтение и выводит его содержимое;
3. перемещает объект `file_read` в `moved_file`;
4. устанавливает позицию чтения в начало файла через `setPosition(0)`;
5. повторно считывает и выводит содержимое уже через перемещённый объект.

### Ожидаемый вывод

```text
Bytes written: 14
File contents: Hello, World!

Moved file contents: Hello, World!
```

---

### Пример запуска

```bash
cd "Paragraph 10"
g++ -Wall -Wextra -O2 01_linux_file_wrapper.cpp -o linux_file_wrapper
./linux_file_wrapper
```

## Проверка закрытия файловых дескрипторов

```bash
valgrind --track-fds=yes ./linux_file_wrapper
```

Эта проверка используется для контроля корректного закрытия файловых дескрипторов после завершения программы.

### Пример запуска

```bash
cd "Paragraph 10"
valgrind --track-fds=yes ./linux_file_wrapper
```

---

## Вывод

В 10 параграфе был реализован класс `File`, инкапсулирующий файловый дескриптор Linux и основные операции ввода-вывода через `open`, `close`, `read`, `write` и `lseek`. Корректность обработки ошибок обеспечивается функцией `check`, а управление ресурсом — деструктором, copy- и move-операциями. В обновлённой версии кода для записи используется вспомогательная функция `writeAll`, чтение выполняется через стандартный буфер `std::vector<char>`, а сборка производится из файла `01_linux_file_wrapper.cpp` в исполняемый файл `linux_file_wrapper`. Дополнительно предусмотрена проверка через `valgrind --track-fds=yes`, позволяющая убедиться в отсутствии незакрытых файловых дескрипторов.

# Отчёт №9

## Цель работы
- Освоить работу с системными вызовами через функции-обёртки libc и универсальную функцию `syscall`.
- На практике использовать `fork`/`exec`/`waitpid` для запуска команды в дочернем процессе.
- Получить доступ к переменным среды через глобальную переменную `environ`.
- Использовать `clone`, `sethostname` и `gethostname` для изоляции имени хоста в дочернем процессе.

## Среда выполнения
- ОС: Ubuntu
- Компилятор: `gcc`
- Инструменты: `strace` (для анализа, опционально)

---

## 1) Системные вызовы: `getpid()` и `syscall(SYS_getpid)`

### Код решения
```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = getpid();
    long pid_syscall = syscall(SYS_getpid);

    printf("PID via getpid: %d\n", pid);
    printf("PID via syscall: %ld\n", pid_syscall);

    if ((long)pid == pid_syscall)
        printf("PIDs are equal\n");
    else
        printf("PIDs are not equal\n");

    return 0;
}
```

### Сборка и запуск
```bash
gcc -Wall -Wextra -O2 01_compare_getpid_methods.c -o compare_getpid_methods
./compare_getpid_methods
```

Ожидаемый результат: значения PID, полученные через `getpid()` и через `syscall(SYS_getpid)`, совпадают.

### Пояснение
- `getpid()` обращается к системному вызову через стандартную библиотечную обёртку.
- `syscall(SYS_getpid)` выполняет тот же вызов напрямую по его номеру.
- Сравнение двух результатов показывает, что в обоих случаях программа получает один и тот же идентификатор текущего процесса.
- Использование типов `pid_t` и `long` делает пример аккуратнее и ближе к сигнатурам используемых функций.

### Пример запуска
```bash
cd "Paragraph 9"
gcc -Wall -Wextra -O2 01_compare_getpid_methods.c -o compare_getpid_methods
./compare_getpid_methods
```

---

## 2) Дочерние процессы: `fork()` + `execlp()` + `waitpid()`

### Код решения
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        execlp("expr", "expr", "2", "+", "2", "*", "2", (char *)NULL);
        perror("execlp");
        return 1;
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
        printf("The child process ended with code: %d\n", WEXITSTATUS(status));
    else
        printf("The child process ended with error\n");

    return 0;
}
```

### Сборка и запуск
```bash
gcc -Wall -Wextra -O2 02_run_expr_in_child_process.c -o run_expr_in_child_process
./run_expr_in_child_process
```

В дочернем процессе запускается команда:
```bash
expr 2 + 2 * 2
```

Ожидаемый вывод `expr` — `6`.

### Пояснение
- `fork()` создаёт копию текущего процесса.
- После `fork()` дочерний процесс определяется по значению `pid == 0`.
- В дочернем процессе `execlp()` заменяет текущий код запуском внешней программы `expr`.
- Родительский процесс вызывает `waitpid(pid, &status, 0)` и ожидает завершения именно этого дочернего процесса.
- Проверка `WIFEXITED(status)` и `WEXITSTATUS(status)` позволяет установить, завершился ли дочерний процесс штатно и с каким кодом.

### Пример запуска
```bash
cd "Paragraph 9"
gcc -Wall -Wextra -O2 02_run_expr_in_child_process.c -o run_expr_in_child_process
./run_expr_in_child_process
```

---

## 3) Переменные среды: вывод переменных, имя которых содержит `L`

### Код решения
```c
#include <stdio.h>

extern char **environ;

static int has_uppercase_l_in_name(const char *str)
{
    while (*str != '\0' && *str != '=')
    {
        if (*str == 'L')
            return 1;
        ++str;
    }

    return 0;
}

int main(void)
{
    for (char **env = environ; *env != NULL; ++env)
    {
        if (has_uppercase_l_in_name(*env))
            printf("%s\n", *env);
    }

    return 0;
}
```

### Сборка и запуск
```bash
gcc -Wall -Wextra -O2 03_print_env_vars_with_L.c -o print_env_vars_with_L
./print_env_vars_with_L
```

Программа проходит по `environ` и выводит только те переменные, у которых **в имени (до символа `=`)** встречается буква `L`.

### Пояснение
- Глобальная переменная `environ` содержит массив строк вида `ИМЯ=ЗНАЧЕНИЕ`.
- Функция `has_uppercase_l_in_name()` проверяет только часть строки до символа `=`, то есть именно имя переменной среды.
- Если в имени встречается символ `L`, строка выводится на экран полностью.
- Таким образом, программа не изменяет окружение, а только выполняет его выборочный просмотр.

### Пример запуска
```bash
cd "Paragraph 9"
gcc -Wall -Wextra -O2 03_print_env_vars_with_L.c -o print_env_vars_with_L
./print_env_vars_with_L
```

---

## 4) Аргументы и пространства имён: `clone()` + `sethostname()` + `gethostname()`

### Код решения
```c
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int child_main(void *ptr)
{
    const char *name = (const char *)ptr;

    if (sethostname(name, strlen(name)) == -1)
    {
        perror("sethostname");
        return 1;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("gethostname");
        return 1;
    }

    printf("Child process: %s\n", hostname);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    const size_t stack_size = 1024 * 1024;
    char *child_stack = malloc(stack_size);
    if (child_stack == NULL)
    {
        perror("malloc");
        return 1;
    }

    pid_t pid = clone(child_main,
                      child_stack + stack_size,
                      CLONE_NEWUTS | CLONE_NEWUSER | SIGCHLD,
                      argv[1]);

    if (pid == -1)
    {
        perror("clone");
        free(child_stack);
        return 1;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("gethostname");
        free(child_stack);
        return 1;
    }

    printf("Parent process: %s\n", hostname);

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        free(child_stack);
        return 1;
    }

    free(child_stack);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}
```

### Сборка и запуск
```bash
gcc -Wall -Wextra -O2 04_isolate_hostname_with_clone.c -o isolate_hostname_with_clone
./isolate_hostname_with_clone spicy
```

Программа создаёт дочерний процесс через `clone()` и в дочернем процессе вызывает `sethostname()`, затем в обоих процессах печатает результат `gethostname()`.

### Пояснение
- `clone()` в данном примере создаёт дочерний процесс с новыми пространствами имён `UTS` и `USER`.
- Флаг `CLONE_NEWUTS` позволяет изолировать hostname дочернего процесса от родительского.
- В `child_main()` имя хоста изменяется функцией `sethostname()`, после чего считывается через `gethostname()`.
- Родительский процесс также вызывает `gethostname()`, поэтому можно сопоставить оба значения.
- В отличие от исходного чернового варианта, здесь добавлены проверки `argc`, `malloc`, `clone`, `gethostname`, `sethostname` и `waitpid`, а также исправлены опечатки в выводе.
- На некоторых системах вызов `clone()` с `CLONE_NEWUSER` и `CLONE_NEWUTS` может завершиться с ошибкой `Operation not permitted`; это связано с настройками пространств имён, а не с ошибкой в логике программы.

### Пример запуска
```bash
cd "Paragraph 9"
gcc -Wall -Wextra -O2 04_isolate_hostname_with_clone.c -o isolate_hostname_with_clone
./isolate_hostname_with_clone spicy
```

---

## Вывод
Были рассмотрены четыре практических случая обращения к механизмам ядра Linux. В первой программе сопоставлены результаты `getpid()` и `syscall(SYS_getpid)`, что подтвердило эквивалентность библиотечной обёртки и прямого системного вызова. Во второй программе отработана схема `fork()` → `execlp()` → `waitpid()`: дочерний процесс выполнил `expr`, а родитель получил код его завершения. В третьем примере через `environ` был выполнен выборочный вывод переменных среды, в имени которых содержится буква `L`. В последней программе с помощью `clone()`, `CLONE_NEWUTS`, `CLONE_NEWUSER`, `sethostname()` и `gethostname()` показано, что имя хоста можно изменить только внутри дочернего пространства имён, не затрагивая родительский процесс.

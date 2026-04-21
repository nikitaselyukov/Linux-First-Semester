# Отчёт №7

## Цель работы
- Инициализировать локальный Git-репозиторий, выполнить начальный коммит и просмотреть историю изменений.
- Освоить базовую работу с ветками: создание новой ветки, фиксацию изменений в ней и последующее слияние с основной веткой.
- Организовать «серверный» bare-репозиторий с помощью `git init --bare`, выполнить отправку истории и проверить корректность клонирования.
- Смоделировать конфликт при изменении одного и того же файла в разных ветках, выполнить `merge` и устранить конфликт вручную.

## Среда выполнения
- ОС: Ubuntu
- Инструменты: `git`, VS Code (для редактирования файлов)

---

## 1) Первый репозиторий: инициализация и первый коммит

### Выполненные команды и результат
```bash
git init
git status
git add "report 7.md"
git commit -m "init"
git log
```

Фрагменты вывода:
```text
[master (корневой коммит) 8c6ad9e] init
 1 file changed, 198 insertions(+)
 create mode 100644 report 7.md

commit 8c6ad9e42c3da1115442c898c60edd2e890fed51
Author: Nikita Selyukov <nikitaselyukov@zohomail.eu>
Date:   Sat Apr 4 19:15:53 2026 +0300

    init
```

### Пример запуска
```bash
cd "Paragraph 7"
git init
git status
git add "report 7.md"
git commit -m "init"
git log
```

---

## 2) Ветки: создание ветки, коммит и слияние

### Выполненные команды и результат
```bash
git checkout -b work
touch new_file.txt
git add new_file.txt
git commit -m "new commit"
git checkout master
git merge work
```

Слияние прошло в режиме fast-forward:
```text
Fast-forward
 new_file.txt | 0
 1 file changed, 0 insertions(+), 0 deletions(-)
 create mode 100644 new_file.txt
```

История коммитов после слияния:
```text
e995b8f (HEAD -> master, work) new commit
8c6ad9e init
```

### Пример запуска
```bash
cd "Paragraph 7"
git checkout -b work
touch new_file.txt
git add new_file.txt
git commit -m "new commit"
git checkout master
git merge work
```

---

## 3) Синхронизация: bare-репозиторий, push и clone

### Создание «серверного» репозитория
В отдельном каталоге был создан bare-репозиторий:
```bash
mkdir -p "/home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare"
cd "/home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare"
git init --bare
```

### Привязка remote и отправка истории на «сервер»
```bash
cd "/home/nik/Linux.FirstSem/Paragraph 7"
git remote remove origin 2>/dev/null
git remote add origin /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare
git push -u origin master
git remote -v
```

Результат push:
```text
To /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare
 * [new branch]      master -> master
branch 'master' set up to track 'origin/master'.

origin  /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare (fetch)
origin  /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare (push)
```

### Клонирование в новую директорию
```bash
cd /home/nik/Linux.FirstSem
git clone /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare SeventhClone
```

После клонирования в рабочем каталоге появились файлы репозитория:
```text
report 7.md  new_file.txt
```

### Пример запуска
```bash
# 1) создать bare-репозиторий
mkdir -p "/home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare"
cd "/home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare"
git init --bare

# 2) из рабочей копии привязать remote и отправить
cd "/home/nik/Linux.FirstSem/Paragraph 7"
git remote remove origin 2>/dev/null
git remote add origin /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare
git push -u origin master

# 3) клонировать вторую копию
cd /home/nik/Linux.FirstSem
git clone /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare SeventhClone
```

---

## 4) Исправление конфликтов: две ветки меняют один и тот же файл

### Изменение файла в первой копии и отправка ветки на сервер
В исходном репозитории:
```bash
cd "/home/nik/Linux.FirstSem/Paragraph 7"
git checkout -b branchA
echo "branchA" > new_file.txt
git add new_file.txt
git commit -m "branchA"
git push -u origin branchA
```

Результат:
```text
[branchA 156d9c9] branchA
 1 file changed, 1 insertion(+)
 * [new branch]      branchA -> branchA
branch 'branchA' set up to track 'origin/branchA'.
```

### Изменение того же файла во второй копии и отправка ветки на сервер
В клонированном репозитории:
```bash
cd "/home/nik/Linux.FirstSem/SeventhClone"
git config user.name "Nikita Selyukov"
git config user.email "nikitaselyukov@zohomail.eu"

git checkout -b branchB
echo "branchB" > new_file.txt
git add new_file.txt
git commit -m "branchB"
git push -u origin branchB
```

Результат:
```text
[branchB 4bcfe4e] branchB
 1 file changed, 1 insertion(+)
 * [new branch]      branchB -> branchB
branch 'branchB' set up to track 'origin/branchB'.
```

### Слияние веток и возникновение конфликта
Далее в клонированном репозитории было выполнено слияние изменений:
```bash
git checkout master
git merge origin/branchA
git merge branchB
```

Подтягивание `origin/branchA` прошло в режиме fast-forward, а при слиянии `branchB` возник конфликт содержимого в `new_file.txt`:
```text
Автослияние new_file.txt
КОНФЛИКТ (добавление/добавление): Конфликт слияния в new_file.txt
Сбой автоматического слияния; исправьте конфликты, затем зафиксируйте результат.
```

### Разрешение конфликта вручную и завершение merge
Конфликтующие маркеры в `new_file.txt` были удалены вручную, а итоговое содержимое файла приведено к виду:
```text
branchA
branchB
```

После этого выполнены команды:
```bash
git add new_file.txt
git commit -m "resolve conflict"
git push

printf "branchA\nbranchB\n" > new_file.txt
git add new_file.txt
git commit -m "fix resolved file content"
git push
```

Результат:
```text
[master b6e3a0e] resolve conflict
[master 0ec2344] fix resolved file content
To /home/nik/Linux.FirstSem/Linux.FirstSem.Etc/SeventhBare
   b6e3a0e..0ec2344  master -> master
```

Итоговое состояние клонированного репозитория:
```bash
git status
git log --oneline --decorate --all
```

```text
nothing to commit, working tree clean

baa251a (HEAD -> master, origin/master) finalize report 7
6edae2a update report 7
0ec2344 fix resolved file content
b6e3a0e resolve conflict
4bcfe4e (origin/branchB, branchB) branchB
156d9c9 (origin/branchA, branchA) branchA
e995b8f (work) new commit
8c6ad9e init
```

### Пример запуска
```bash
# в первой копии
cd "/home/nik/Linux.FirstSem/Paragraph 7"
git checkout -b branchA
echo "branchA" > new_file.txt
git add new_file.txt
git commit -m "branchA"
git push -u origin branchA

# во второй копии
cd "/home/nik/Linux.FirstSem/SeventhClone"
git checkout -b branchB
echo "branchB" > new_file.txt
git add new_file.txt
git commit -m "branchB"
git push -u origin branchB

# слияние и фиксация конфликта
git checkout master
git merge origin/branchA
git merge branchB
```

---

## Вывод
В ходе работы был создан локальный Git-репозиторий, настроены имя автора и адрес электронной почты, после чего выполнен начальный коммит. Затем была создана ветка `work`, добавлен файл `new_file.txt` и выполнено fast-forward слияние с веткой `master`. После этого был создан bare-репозиторий `SeventhBare`, настроен `origin`, выполнены `push` и `clone`, что позволило смоделировать обмен изменениями между двумя рабочими копиями. На заключительном этапе в ветках `branchA` и `branchB` были сделаны разные изменения одного и того же файла, возник конфликт слияния, который сначала был разрешён вручную, а затем итоговое содержимое файла было дополнительно зафиксировано коммитом `fix resolved file content`. Финальная версия изменений была отправлена в bare-репозиторий.

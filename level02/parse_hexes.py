from typing import List


def do_parse(hexes: List[str]) -> str:
    res = ''
    for one in hexes:
        one = one.strip('0x')
        b = bytes.fromhex(one)
        res += b.decode('utf-8')[::-1]  # reverse it because data in little endian
    return res

if __name__ == '__main__':
    f_name = input('pass file name (with hexes saved line by line): ')
    with open(f_name, 'r') as f:
        args = f.read().split()
    password = do_parse(args)
    print(password)


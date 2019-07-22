import re

line = input()

prev = 0
for m in re.finditer(r'0x([A-Fa-f0-9]+):0x([A-Fa-f0-9]+)', line):
	pc = int(m.group(1), 16)
	sp = int(m.group(2), 16)
	print(hex(pc), ":", hex(sp), "+", sp - prev)
	prev = sp

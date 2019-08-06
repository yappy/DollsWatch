import sys
import re

line = input()

addrs=[]
prev = 0
for m in re.finditer(r'0x([A-Fa-f0-9]+):0x([A-Fa-f0-9]+)', line):
	pc = int(m.group(1), 16)
	sp = int(m.group(2), 16)
	addrs.append({ "pc": pc, "sp": sp, "diff": sp - prev })
	prev = sp

symbols = []
for line in sys.stdin:
	m = re.match(r'0x([A-Fa-f0-9]+):\s*(.*)', line)
	if (m):
		addr = int(m.group(1), 16)
		name = m.group(2)
		symbols.append({ "addr": addr, "name": name })

for addr in addrs:
	sym = next((x for x in symbols if addr["pc"] == x["addr"]), {"name": "????????"})
	print(addr["pc"], ":", addr["sp"], "+", addr["diff"])
	print(sym["name"])

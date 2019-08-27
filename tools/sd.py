import os
import sys
import glob
import pathlib
import requests

def upload(arg):
	src_dir = pathlib.Path(arg[0]).resolve()
	dst_dir = arg[1] if len(arg) >= 2 else '/sd/'
	print('src_root=', src_dir)
	# Get all files under src_dir/
	all = list(src_dir.glob("**/*"))
	for entry in all:
		cmd = None
		if entry.is_dir():
			cmd = 'MKDIR'
		elif entry.is_file():
			cmd = 'UPLOAD'
		else:
			print('?')
			continue
		print(entry)
		# Relative to src_dir, force separator '/'
		target_path = dst_dir + \
			str(entry.relative_to(src_dir)).replace(os.sep, '/')
		print('->', cmd, target_path)

cmd_table = {
	'upload': upload
}

def main(argv):
	if len(argv) < 2:
		print('sd.py <ipaddr> <cmd> ...')
		exit(1)
	ipaddr = argv[1]
	cmd = argv[2]
	arg = argv[3:]
	if not cmd_table[cmd]:
		print('Command not found: {cmd}'.format(cmd=cmd))
	cmd_table[cmd](arg)

main(sys.argv)

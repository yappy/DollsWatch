import os
import sys
import glob
import pathlib
import requests

# >= python 3.5
assert sys.version_info[0] > 3 or \
	(sys.version_info[0] >= 3 and sys.version_info[1] >= 5)

def check_resp(resp):
	try:
		resp.raise_for_status()
	except requests.exceptions.HTTPError as e:
		e.args += (resp.text,)
		raise

def list_cmd(url, arg):
	headers = {
		'FILE-CMD': 'LIST'
	}
	resp = requests.get(url + 'file', headers=headers)
	check_resp(resp)
	target_files = resp.json()
	target_files.sort()
	for path in target_files:
		print(path)

def upload_cmd(url, arg):
	src_dir = pathlib.Path(arg[0]).resolve()
	dst_dir = arg[1] if len(arg) >= 2 else '/sd/'
	print('src_root=', src_dir)

	# Get file list in the target
	headers = {
		'FILE-CMD': 'LIST'
	}
	resp = requests.get(url + 'file', headers=headers)
	check_resp(resp)
	target_files = resp.json()

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

		if cmd == 'MKDIR':
			if not target_path + '/' in target_files:
				headers = {
					'FILE-CMD': 'MKDIR',
					'FILE-PATH': target_path,
				}
				resp = requests.post(url + 'file', headers=headers)
				check_resp(resp)
				print('OK')
			else:
				print('SKIP (already exist)')
		elif cmd == 'UPLOAD':
			headers = {
				'Content-Type': 'application/octet-stream',
				'FILE-CMD': 'UPLOAD',
				'FILE-PATH': target_path,
			}
			with entry.open(mode='rb') as f:
				resp = requests.post(url + 'file', data=f, headers=headers)
			check_resp(resp)
			print('OK')

def initialize_cmd(url, arg):
	headers = {
		'FILE-CMD': 'LIST'
	}
	resp = requests.get(url + 'file', headers=headers)
	check_resp(resp)
	target_files = resp.json()
	target_files.sort()

	# Delete all files at first
	for entry in target_files:
		if not entry.endswith('/'):
			print('Delete file:', entry)
			headers = {
				'FILE-CMD': 'DELETE',
				'FILE-PATH': entry,
			}
			resp = requests.post(url + 'file', headers=headers)
			check_resp(resp)
	# Then delete all directories
	for entry in target_files:
		if entry.endswith('/'):
			print('Delete dir:', entry)
			headers = {
				'FILE-CMD': 'DELETE',
				# Must remove the last '/'
				'FILE-PATH': entry.rstrip('/'),
			}
			resp = requests.post(url + 'file', headers=headers)
			check_resp(resp)

cmd_table = {
	'list'			: list_cmd,
	'upload'		: upload_cmd,
	'initialize'	: initialize_cmd,
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
	url = 'http://' + ipaddr + '/recovery/'
	cmd_table[cmd](url, arg)

main(sys.argv)

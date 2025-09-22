#!/usr/bin/python3

import cgi
import os
import sys
import cgitb
cgitb.enable() 
# --> According to python.org : "This activates a special exception handler that will display 
# detailed reports in the web browser if any errors occur."

form = cgi.FieldStorage()


# def log(msg):
# 	script_dir = os.path.dirname(os.path.abspath(__file__))

# 	if not os.path.exists(log_path):
# 			os.makedirs(log_path)
# 	log_dir = os.path.join(script_dir, '..', 'debugfiles', 'logs')
# 	log_path = os.path.join(log_dir, "cgi_upload_debug.log")
# 	if not os.path.exists(log_path):
# 			os.makedirs(log_path)
# 	with open(log_path, 'a') as l:
# 		l.write(msg + '\n')

def uploadNewFile():

	# cpt = 1

	# while 1:
	# 	cpt = cpt + 1

	fileitem = form['myFilePicker']
	if fileitem.filename:
		# log("File was uploaded: " + str(fileitem.filename))
		script_dir = os.path.dirname(os.path.abspath(__file__))
		upload_dir = os.path.join(script_dir, '..', 'server_files', 'uploads') #use actual names
		if not os.path.exists(upload_dir):
			os.makedirs(upload_dir)

		filename = os.path.basename(fileitem.filename)
		parsed_name = filename.replace(" ", "_")
		path = os.path.join(upload_dir, parsed_name)
		data = fileitem.file.read()
		with open(path, 'wb') as f:
			f.write(data)

uploadNewFile()
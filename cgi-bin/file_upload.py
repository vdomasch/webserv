#!/usr/bin/python3

import cgi
import os
import sys
import cgitb
cgitb.enable() 
# --> According to python.org : "This activates a special exception handler that will display 
# detailed reports in the web browser if any errors occur."

# form = cgi.FieldStorage()


# def log(msg):
# 	script_dir = os.path.dirname(os.path.abspath(__file__))
# 	log_dir = os.path.join(script_dir, '..', 'debugfiles', 'logs')
# 	log_path = os.path.join(log_dir, "cgi_upload_debug.log")
# 	with open(log_path, 'a') as l:
# 		l.write(msg + '\n')

# def printValueToPage():

# 	fileitem = form['filePicker']
# 	log("fileitem: %s" % repr(fileitem))
# 	if fileitem.filename:
# 		log("filename: %s" % fileitem.filename)
# 		script_dir = os.path.dirname(os.path.abspath(__file__))
# 		upload_dir = os.path.join(script_dir, '..', 'server_files', 'upload')
# 		filename = os.path.basename(fileitem.filename)
# 		path = os.path.join(upload_dir, filename)
# 		data = fileitem.file.read()
# 		log("data length: %d" % len(data))
# 		with open(path, 'wb') as f:
# 			f.write(data)
# 		log("saved to: %s" % path)
# 	else:
# 		log("No filename found in fileitem.")

print ('<html>')
print ('<head>')
print ('<title>File analysis O-O</title>')
print ('</head>')
print ('<body>')
print ('<h2>Time for some request analysis !\nHere i come !</h2>')

# printValueToPage()

print ('<form  method = "post" action = "/index.html">')
print ('<button name="goto_index" value="teleport">Go back to main page</button>')
print ('</form>')
print ('</body>')
print ('</html>')


# -----------------------------------------------------------------------------------------------


# # for key, value in os.environ.items():
# # 	print(f"{key}={value}\n")

# try:
# 	content_length = int(os.environ.get("CONTENT_LENGTH", 0))
# except ValueError:
# 	sys.exit([-1])

# data = sys.stdin.buffer.read(content_length)

# # script_dir = os.path.dirname(os.path.abspath(__file__))
# # log_dir = os.path.join(script_dir, '..', 'debugfiles', 'logs')
# # # print(f"\nScript dir : {script_dir}\n")
# # # print(f"\nLog dir : {log_dir}\n")

# # log_path = os.path.join(log_dir, "cgi_upload_debug.bin")

# # with open(log_path, "wb") as f:
# # 	f.write(data)

# print(f"Read {len(data)} bytes (expected: {content_length})")
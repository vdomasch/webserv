#!/usr/bin/python3

import cgi
import os
import cgitb
cgitb.enable() 
# --> According to python.org : "This activates a special exception handler that will display 
# detailed reports in the web browser if any errors occur."

form = cgi.FieldStorage()


def printValueToPage():
	fileitem = form['filePicker']
	if fileitem.filename:
		script_dir = os.path.dirname(os.path.abspath(__file__))
		upload_dir = os.path.join(script_dir, '..', 'server_files', 'upload')
		filename = os.path.basename(fileitem.filename)
		path = os.path.join(upload_dir, filename)
		with open(path, 'wb') as f:
			f.write(fileitem.file.read())

print ('<html>')
print ('<head>')
print ('<title>File analysis O-O</title>')
print ('</head>')
print ('<body>')
print ('<h2>Time for some request analysis !\nHere i come !</h2>')

printValueToPage()

print ('<form  method = "post" action = "/index.html">')
print ('<button name="goto_index" value="teleport">Go back to main page</button>')
print ('</form>')
print ('</body>')
print ('</html>')
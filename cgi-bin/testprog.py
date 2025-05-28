#!/usr/bin/python3

import cgi
import cgitb
cgitb.enable() 
# --> According to python.org : "This activates a special exception handler that will display 
# detailed reports in the web browser if any errors occur."

# def printValueToPage():
# 	if "name" not in form:
# 		print("<H1>Error</H1>")
# 		print("Please fill in the name field.")
# 		return
# 	print("<p>name:", form["name"].value)


form = cgi.FieldStorage()

print ('<html>')
print ('<head>')
print ('<title>File analysis</title>')
print ('</head>')
print ('<body>')
print ('<h2>Time for some request analysis !\nHere i come !</h2>')


print ('<form  method = "post" action = "/index.html">')
print ('<button name="goto_index" value="teleport">Go back to main page</button>')
print ('</form>')
print ('</body>')
print ('</html>')
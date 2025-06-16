#!/usr/bin/python3

int i = 0
while true:
	i = 4


print ('<!DOCTYPE html>')
print ('<html>')
print ('<head>')
print ('<link rel="icon" type="image/x-icon" href="/icons/favicon.ico">')
print ('<meta charset="UTF-8">')
print ('<meta name="viewport" content="width=device-width, initial-scale=1.0">')	
print ('<title>File upload</title>')
print ('</head>')
print ('<body style="background-color: #bee8fd;">')
print ('<section>')
print ('<div class="container" style="text-align: center; margin-top: 6em; width: 33%; margin-left: auto; margin-right: auto; font-family: Calibri; font-size: 20px; line-height: 1.5; padding: 2em; border: 1px solid #ccc; border-radius: 5px; background-color: #f9f9f9; box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);">') 
print ('<h3>Silly goofy file upload</h3>')
print ('<img src="/assets/pinwheel.gif" alt="dog" style="display:block; margin:auto; max-width:300px; max-height:300px;">')
print ('<p>(me irl btw)</p>')
print ('<label for="myFilePicker">Please select a file:</label>')

print ('<iframe name="hiddenFrame" style="display:none;"></iframe>')
print ('<form  method = "post" enctype="multipart/form-data" action = "/cgi-bin/file_upload.py" target="hiddenFrame" onsubmit="sendPost();" >')
print ('<input type="file" id="myFilePicker" name="filePicker">')
print ('<input type = "submit" value = "Submit"/>')
print ('</form>')
print ('<p id="uploadResult" style="margin-top: 1em;"></p>')

print ('<form  method = "get" action = "/index.html">')
print ('<button>Go back to main page</button>')
print ('</form>')
print ('</div>')
print ('</section>')
print ('</body>	')
print ('</html>')


print ('<script>')
print ('function sendPost() {')
print ('const fileInput = document.getElementById("myFilePicker");')
print ('const file = fileInput.files[0];')
print ('if (!file) {')
print ('document.getElementById("uploadResult").innerText = "❌ Please select a file.";')
print ('return;')
print ('}')
print ('const formData = new FormData();')
print ('formData.append("myFilePicker", file);')
print ('')
print ('fetch("/cgi-bin/file_upload.py", {')
print ('method: "POST",')
print ('body: formData')
print ('})')
print ('.then(response => {')
print ('return response.text().then(text => {')
print ('if (response.ok) {')
print ('document.getElementById("uploadResult").innerText = "✅ File send";')
print ('} else {')
print ('document.getElementById("uploadResult").innerText = "❌ Error ${response.status} : ${text}";')
print ('}')
print ('fileInput.value = "";')
print ('});')
print ('})')
print ('.catch(error => {')
print ('document.getElementById("uploadResult").innerText = "❌ Error network : " + error;')
print ('});')
print ('}')
print ('</script>')




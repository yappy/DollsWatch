local html = [[
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" />
  <title>Hello</title>
</head>
<body>
  <p>Hello my watch!!!!!</p>
  <p><a href="./files">File Server</a></p>
</body>
</html>
]]

print(html)

return 200, "text/html", html

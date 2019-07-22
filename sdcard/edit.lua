local html = [[
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" />
  <title>Edit file</title>
</head>
<body>
  <form action="POST">
    <textarea name="data" rows="30" cols="80"></textarea>
  </form>
</body>
</html>
]]

print(html)

return "200 OK", "text/html", html

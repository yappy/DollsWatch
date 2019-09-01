local common = require("common")
local webutil = require("webutil")

local GET_DATA = [[
<!DOCTYPE html>
<html>
<head>
  <title></title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <form>
    <textarea id = "data" autofocus rows="20" cols="80"></textarea>
    <div><input id="submit" type="button" value="Submit" /></div>
    <textarea id = "output" rows="20" cols="80"></textarea>
  </form>

<script>
  var post_file = function(data) {
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '?p=exec', false);
    xhr.setRequestHeader('Content-type', "text/plain");
    xhr.send(data);
    document.getElementById("output").value = xhr.responseText;
  }

  document.getElementById("submit").addEventListener(
    "click",
    function() {
      var data = document.getElementById("data").value;
      post_file(data);
    },
    false);
</script>

</body>
</html>
]]

local function get(lua_root, query)
	return webutil.response(200, "text/html", nil, GET_DATA)
end

local function post(lua_root, query, content_length, recv)
	local src = ""
	while content_length > 0 do
		local size = math.min(common.DEF_BUF_SIZE, content_length)
		local buf = recv(size)
		src = src .. buf
		content_length = content_length - #buf
	end

	local func, syn_err = load(src)
	if not func then
		return webutil.response(200, "text/plain", nil, syn_err)
	end

	local output = ""
	local orig_print = print
	print = function(...)
		local args = table.pack(...)
		for i = 1, args.n do
			if i ~= 1 then
				output = output .. "\t"
			end
			output = output .. tostring(args[i])
		end
		output = output .. "\n"
	end
	local status, run_err = pcall(func)
	if status then
		return webutil.response(200, "text/plain", nil, output)
	else
		return webutil.response(200, "text/plain", nil, run_err)
	end
end

local lua_root, method, query, content_length, recv = ...
if method == "GET" then
	return get(lua_root, query)
elseif method == "POST" then
	return post(lua_root, query, content_length, recv)
else
	return webutil.response(405)
end

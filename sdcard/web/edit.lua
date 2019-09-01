local common = require("web.common")
local webutil = require("web.webutil")

local GET_TEMPLATE = [[
<!DOCTYPE html>
<html>
<head>
  <title></title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <form>
    <textarea id = "data" autofocus rows="40" cols="80">@@text@@</textarea>
    <div><input id="submit" type="button" value="Submit" /></div>
  </form>

<script>
  var post_file = function(file_name, data) {
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '?p=edit&f=' + file_name, false);
    xhr.setRequestHeader('Content-type', "text/plain");
    xhr.send(data);
    location.reload(true);
  }

  document.getElementById("submit").addEventListener(
    "click",
    function() {
      var file_name = "@@file_url@@";
      var data = document.getElementById("data").value;
      post_file(file_name, data);
    },
    false);
</script>

</body>
</html>
]]

local function is_valid_filename(name)
	return string.match(name, "[%w]*%.[%w]+")
end

local function get(lua_root, query)
	if not query["f"] then
		return webutil.response(400, "text/plain", nil, "File name required")
	end

	local file_name = query["f"]
	local f = io.open(file_name, "r")
	if not f then
		return webutil.response(400, "text/plain", nil, "Cannot open")
	end

	webutil.response_header(200, "text/html", nil, data)
	local func_tbl = {}
	function func_tbl.text()
		while true do
			local data = f:read(common.DEF_BUF_SIZE)
			if not data then
				break
			end
			coroutine.yield(webutil.html_escape(data))
		end
	end
	function func_tbl.file_url()
		coroutine.yield(webutil.escape(file_name))
	end
	webutil.response_body(GET_TEMPLATE, func_tbl)
	f:close()
	return
end

local function post(lua_root, query, content_length, recv)
	print("post!")
	while content_length > 0 do
		local size = math.min(common.DEF_BUF_SIZE, content_length)
		local buf = recv(size)
		io.write(buf)
		content_length = content_length - #buf
	end
end

local lua_root, method, query, content_length, recv = ...
if method == "GET" then
	return get(lua_root, query)
elseif method == "POST" then
	post(lua_root, query, content_length, recv)
	return webutil.response(500, "text/plain", nil, "Not implemented")
else
	return webutil.response(405)
end

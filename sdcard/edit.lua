local webutil = require("webutil")

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
    <textarea autofocus rows="40" cols="80">@@text@@</textarea>
  </form>
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

	local f = io.open(lua_root .. query["f"], "r")
	if not f then
		return webutil.response(400, "text/plain", nil, "Cannot open")
	end

	webutil.response_header(200, "text/plain", nil, data)
	local func_tbl = {}
	function func_tbl.text()
		while true do
			local data = f:read(256)
			if not data then
				break
			end
			coroutine.yield(webutil.html_escape(data))
		end
	end
	webutil.response_body(GET_TEMPLATE, func_tbl)
	f:close()
	return
end

local function post()

end

local lua_root, method, query, content_length, recv = ...
if method == "GET" then
	return get(lua_root, query)
elseif method == "POST" then
	return webutil.response(405)
else
	return webutil.response(405)
end

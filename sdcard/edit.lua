local webutil = require("webutil")

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
	local data = f:read("a")
	f:close()

	return webutil.response(200, "text/plain", nil, data)
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

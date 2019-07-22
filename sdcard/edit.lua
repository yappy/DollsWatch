local webutil = require("webutil")
local method, query, content_length, recv = ...

local function get()

end

local function post()

end

if method == "GET" then
	return webutil.response(200, "text/html", nil, "edit post")
elseif method == "POST" then
	return webutil.response(405)
else
	return webutil.response(405)
end

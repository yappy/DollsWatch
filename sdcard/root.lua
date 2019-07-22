local webutil = require("webutil")

-- query string to table
local function parse_query(query)
	local result = {}
	-- split with '&' (not & 1+ times)
	for ent in string.gmatch(query, "([^&]+)") do
		-- split with '=' (=value can be absent)
		local key, val = string.match(ent, "^([^=]*)=?(.*)$")
		result[webutil.unescape(key)] = webutil.unescape(val)
	end
	return result
end

-- [A-Z, a-z, 0-9, -, _]+
local function is_valid_page(page)
	return string.match(page, "^[%w%-%_]+$") ~= nil
end


--[[
global export main (yieldable)

@returns string http_status, content_type
@returns (n >= 0 times) string header_key, string header_value
@returns void
@returns (n >= 0 times) string response_body
@returns void
]]
function main(lua_root, method, query_str, content_length, recv)
	assert(coroutine.isyieldable())

	local query = parse_query(query_str)
	local page = query["p"] or "index"

	if not is_valid_page(page) then
		print("invalid page")
		return webutil.response(404)
	end

	-- load lua_root/[page].lua
	local name = lua_root .. page .. ".lua"
	local f = io.open(name, "r")
	if not f then
		return webutil.response(404)
	end
	chunk, err = load(function() return f:read("a") end, name, "t")
	f:close()
	if not chunk then
		print(err)
		return webutil.response(500)
	end
	-- call chunk with parameters
	return chunk(method, query, content_length, recv)
end

-- test main
print("WEBAPP:", _ENV.WEBAPP)
print("path ", package.path)
print("cpath", package.cpath)

if not _ENV.WEBAPP then
	local line = io.read("l")
	local th = coroutine.wrap(main)
	while true do
		local ret, ret2 = th("./", "GET", line, 0, nil)
		if ret then
			print(ret, ret2)
		else
			break
		end
	end
	while true do
		local ret, ret2 = th("./", "GET", line, 0, nil)
		if ret then
			print(ret, ret2)
		else
			break
		end
	end
end

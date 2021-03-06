local webutil = require("web.webutil")

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
main coroutine (yieldable)

@returns string http_status, content_type
@returns (n >= 0 times) string header_key, string header_value
@returns void
@returns (n >= 0 times) string response_body
@returns void
]]
local function co_main(lua_root, method, query_str, content_length, recv)
	assert(coroutine.isyieldable())

	local query = parse_query(query_str)
	local page = query["p"] or "index"

	if not is_valid_page(page) then
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
	return chunk(lua_root, method, query, content_length, recv)
end

--[[
global export init, loop
]]
local co = nil
function init()
	assert(co == nil)
	co = coroutine.wrap(co_main)
end

function loop(lua_root, method, query_str, content_length, recv)
	assert(co ~= nil)
	return co(lua_root, method, query_str, content_length, recv)
end

-- test main
print("[Lua web start]", "WEBAPP:", _ENV.WEBAPP)

if not _ENV.WEBAPP then
	init()
	local q = ...
	q = q or io.read("l")
	print("---- HEADER ----")
	while true do
		local ret, ret2 = loop("./", "GET", q, 0, nil)
		if ret then
			print(ret, ret2)
		else
			break
		end
	end
	print("----  BODY  ----")
	while true do
		local ret = loop("./", "GET", q, 0, nil)
		if ret then
			io.write(ret)
		else
			break
		end
	end
end

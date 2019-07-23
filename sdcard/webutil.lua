local webutil = {}

local STATUS_TABLE = {
	[200] = "200 OK",
	[400] = "400 Bad Request",
	[403] = "403 Forbidden",
	[404] = "404 Not Found",
	[405] = "405 Method Not Allowed",
}

-- body is nullable (automatically status meeesge will be used)
function webutil.response(code, content_type, header, body)
	webutil.response_partial(code, content_type, header)
	-- body
	coroutine.yield(body)
	-- void
	return
end

function webutil.response_header(code, content_type, header)
	-- body default = status
	local status = STATUS_TABLE[code] or tostring(code)
	content_type = content_type or "text/html"
	header = header or {}
	body = body or status

	-- status, content_type
	print("yield", status, content_type)
	coroutine.yield(status, content_type)
	-- { key, value } * N
	for k, v in pairs(header) do
		coroutine.yield(k, v)
	end
	-- void
	coroutine.yield()
end

-- template replacing output
function webutil.response_body(tmpl, func_tbl)
	local cur = 1
	for pos1, sym, pos2 in string.gmatch(tmpl, "()@@([^@]+)@@()") do
		-- output cur ~ left of @@
		coroutine.yield(string.sub(tmpl, cur, pos1 - 1))
		-- call func by symbol string
		func_tbl[sym]();
		-- move to right of @@
		cur = pos2
	end
	local rest = string.sub(tmpl, cur)
	if #rest > 0 then
		coroutine.yield(rest)
	end
	-- body end
	coroutine.yield()
end

-- RFC 3986
-- PHP rawurlencode compatible
function webutil.escape(src)
	return (string.gsub(src, "([^A-Za-z0-9-_.~])", function(c)
		return string.format("%%%02x", string.byte(c))
	end))
end

-- + -> space
-- %hh -> raw byte
function webutil.unescape(src)
	src = string.gsub(src, "%+", " ")
	src = string.gsub(
		src,
		"%%(%x%x)",
		function(hex)
			return string.char(tonumber(hex, 16))
		end)
	return src
end

local HTML_ESC = {
	["&"] = "&amp;",
	['"'] = "&quot;",
	["'"] = "&apos;",
	["<"] = "&lt;",
	[">"] = "&gt",
}
function webutil.html_escape(src)
	local result = string.gsub(src, "[%&%\"%'%<%>]", HTML_ESC)
	return result
end

return webutil

local webutil = {}

local STATUS_TABLE = {
	[200] = "200 OK",
	[400] = "400 Bad Request",
	[400] = "403 Forbidden",
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

function webutil.response_partial(code, content_type, header)
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

return webutil

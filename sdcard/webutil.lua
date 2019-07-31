local util = require("util")
local webutil = {}

local STATUS_TABLE = {
	[200] = "200 OK",
	[400] = "400 Bad Request",
	[403] = "403 Forbidden",
	[404] = "404 Not Found",
	[405] = "405 Method Not Allowed",
}

--[[
Return all HTTP response at a time.

int code HTTP status code
string(nilable) content_type HTTP content type
table<string, string>(nilable) header Other HTTP headers table
string body HTTP response body
]]
function webutil.response(code, content_type, header, body)
	util.checkarg(body, "string", true)
	-- body default = status
	body = body or STATUS_TABLE[code]
	body = body or tostring(code)
	-- append LF if body is not empty AND the last character is not LF
	if body ~= "" then
		if string.sub(body, #body) ~= "\n" then
			body = body .. "\n"
		end
	end

	webutil.response_header(code, content_type, header)
	-- body
	coroutine.yield(body)
	-- void
	return
end

--[[
Return HTTP response header.

int code HTTP status code
string(nilable) content_type HTTP content type
table<string, string>(nilable) header Other HTTP headers table
]]
function webutil.response_header(code, content_type, header)
	util.checkarg(code, "number")
	util.checkarg(content_type, "string", true)
	util.checkarg(header, "table", true)
	-- content_type, header default
	local status = STATUS_TABLE[code] or tostring(code)
	content_type = content_type or "text/plain"
	header = header or {}

	-- status, content_type
	coroutine.yield(status, content_type)
	-- { key, value } * N
	for k, v in pairs(header) do
		util.checkarg(k, "string")
		util.checkarg(v, "string")
		coroutine.yield(k, v)
	end
	-- void
	coroutine.yield()
end

--[[
Return HTTP response body.
Non-special part in the template string will be yield-returned.
If @@SYMBOL@@ is found in the template string,
func_tbl["SYMBOL"] will be called.
Please yield-return a substitute string from the function.
If func_tbl is nil, template replacement will be disabled.

string tmpl Template text
table<string, function>(nilable) func_tbl Symbol->Function table
]]
function webutil.response_body(tmpl, func_tbl)
	util.checkarg(tmpl, "string")
	util.checkarg(func_tbl, "table", true)

	if func_tbl == nil then
		coroutine.yield(tmpl)
		return
	end

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

--[[
RFC 3986
PHP rawurlencode compatible
]]
function webutil.escape(src)
	util.checkarg(src, "string")

	return string.gsub(
		src, "([^A-Za-z0-9-_.~])",
		function(c)
			return string.format("%%%02x", string.byte(c))
		end
	)
end

--[[
+ -> space
%hh -> raw byte
]]
function webutil.unescape(src)
	util.checkarg(src, "string")

	src = string.gsub(src, "%+", " ")
	src = string.gsub(
		src, "%%(%x%x)",
		function(hex)
			return string.char(tonumber(hex, 16))
		end
	)
	return src
end

--[[
HTML special characters -> &xxx;
]]
function webutil.html_escape(src)
	util.checkarg(src, "string")

	return string.gsub(src, "[%&%\"%'%<%>]", HTML_ESC)
end

local HTML_ESC = {
	["&"] = "&amp;",
	['"'] = "&quot;",
	["'"] = "&apos;",
	["<"] = "&lt;",
	[">"] = "&gt",
}

local MINE_LIST = {
	["aac"]		=	"audio/aac",
	["avi"]		=	"video/x-msvideo",
	["bmp"]		=	"image/bmp",
	["bz"]		=	"application/x-bzip",
	["bz2"]		=	"application/x-bzip2",
	["css"]		=	"text/css",
	["csv"]		=	"text/csv",
	["doc"]		=	"application/msword",
	["docx"]	=	"application/vnd.openxmlformats-officedocument.wordprocessingml.document",
	["gif"]		=	"image/gif",
	["htm"]		=	"text/html",
	["html"]	=	"text/html",
	["ico"]		=	"image/vnd.microsoft.icon",
	["jar"]		=	"application/java-archive",
	["jpeg"]	=	"image/jpeg",
	["jpg"]		=	"image/jpeg",
	["js"]		=	"text/javascript",
	["json"]	=	"application/json",
	["mid"]		=	"audio/x-midi",
	["midi"]	=	"audio/x-midi",
	["mp3"]		=	"audio/mpeg",
	["mpeg"]	=	"video/mpeg",
	["png"]		=	"image/png",
	["pdf"]		=	"application/pdf",
	["ppt"]		=	"application/vnd.ms-powerpoint",
	["pptx"]	=	"application/vnd.openxmlformats-officedocument.presentationml.presentation",
	["rar"]		=	"application/x-rar-compressed",
	["sh"]		=	"application/x-sh",
	["tar"]		=	"application/x-tar",
	["txt"]		=	"text/plain",
	["wav"]		=	"audio/wav",
	["xhtml"]	=	"application/xhtml+xml",
	["xls"]		=	"application/vnd.ms-excel",
	["xlsx"]	=	"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
	["xml"] 	=	"application/xml",
	["zip"]		=	"application/zip",
	["7z"]		=	"application/x-7z-compressed",
}

return webutil

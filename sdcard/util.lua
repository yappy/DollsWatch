local util = {}

function util.checkarg(val, expect_type, nilable)
	assert(type(expect_type) == "string")
	local actual_type = type(val)

	if nilable and actual_type == "nil" then
		return
	end
	if actual_type ~= expect_type then
		error(expect_type .. " expected, got " .. actual_type, 2)
	end
end

return util

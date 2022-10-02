-- This lua script contains various common functionality
-- These can be used in the different lua files by the global called core

local core = {}

-- Convert bool to int
function core.btoi(b)
	return b and 1 or 0
end

return core
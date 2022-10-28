-- This lua script contains various common functionality
-- These can be used in the different lua files by the global called core

local core = {}

-- Some default colors
core.RED = vector(1, 0, 0)
core.GREEN = vector(0, 1, 0)
core.BLUE = vector(0, 0, 1)
core.YELLOW = vector(1, 1, 0)
core.MAGENTA = vector(1, 0, 1)
core.CYAN = vector(0, 1, 1)
core.BLACK = vector(0, 0, 0)
core.WHITE = vector(1, 1, 1)
core.GRAY = vector(0.5, 0.5, 0.5)
core.LIGHTGRAY = vector(0.8, 0.8, 0.8)

-- Convert bool to int
function core.btoi(b)
	return b and 1 or 0
end

return core
local vector = {}

function vector.new(x, y, z)
	local t = {
		x = x or 0,
		y = y or 0,
		z = z or 0
	}
	setmetatable(t, vector)
	return t
end

function vector.fill(v)
	return vector.new(v, v, v)
end

function vector.isvector(t)
	return getmetatable(t) == vector
end

function vector.__newindex(t, k, v)
	print("Can't create more fields in vectors")
end

vector.__index = vector

function vector.__tostring(t)
	return "(" .. t.x .. ", " .. t.y .. ", " .. t.z .. ")"
end

function vector.__unm(t)
	return vector.new(-t.x, -t.y, -t.z)
end

function vector.__add(a, b)
	if (not vector.isvector(a) or not vector.isvector(b)) then
		return
	end
	
	return vector.new(a.x + b.x, a.y + b.y, a.z + b.z)
end

function vector.__sub(a, b)
	if (not vector.isvector(a) or not vector.isvector(b)) then
		return
	end
	
	return vector.new(a.x - b.x, a.y - b.y, a.z - b.z)
end

function vector.__mul(a, b)
	if (vector.isvector(a) and type(b) == "number") then
		return vector.new(a.x * b, a.y * b, a.z * b)
	elseif (vector.isvector(b) and type(a) == "number") then
		return vector.new(a * b.x, a * b.y, a * b.z)
	elseif (vector.isvector(a) and vector.isvector(b)) then
		return vector.new(a.x * b.x, a.y * b.y, a.z * b.z)
	end
end

function vector.__div(a, b)
	if (vector.isvector(a) and type(b) == "number") then
		return vector.new(a.x / b, a.y / b, a.z / b)
	elseif (vector.isvector(a) and vector.isvector(b)) then
		return vector.new(a.x / b.x, a.y / b.y, a.z / b.z)
	end
end

function vector.__eq(a, b)
	if (not vector.isvector(a) or not vector.isvector(b)) then
		return false
	end

	return a.x == b.x and a.y == b.y and a.z == b.z
end

function vector:length()
	return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)
end

function vector:normalize()
	local l = self:length()
	if(l == 0) then
		return vector.new(0, 0, 0)
	else
		return self / l
	end
end

return setmetatable(vector, {
	__call = function(_, ...)
	return vector.new(...)
end
})
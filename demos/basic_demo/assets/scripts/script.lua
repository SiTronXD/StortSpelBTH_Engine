local script = {}

function script:init()
	print("init with ID: " .. self.ID)
	self.speed = 5
end

function script:update(dt)
	local x = core.btoi(input.isKeyDown(Keys.A)) - core.btoi(input.isKeyDown(Keys.D))
	local z = core.btoi(input.isKeyDown(Keys.W)) - core.btoi(input.isKeyDown(Keys.S))
	local moveVec = vector(x, 0, z)

	self.transform.position = self.transform.position + moveVec * dt * self.speed
end

return script;
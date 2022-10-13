local script = {}

function script:init()
	print("init with ID: " .. self.ID)
	print("Script2")
end

local pols = {
	vector(-1,0,-1),
	vector(1,0,-1),
	vector(-1,0,1),
	vector(1,0,1),
	vector(0,0,5),
}

function script:update(dt)
	--local x = core.btoi(input.isKeyDown(Keys.A)) - core.btoi(input.isKeyDown(Keys.D))
	--local z = core.btoi(input.isKeyDown(Keys.W)) - core.btoi(input.isKeyDown(Keys.S))
	--local moveVec = vector(x, 0, z)

	--self.transform.position = self.transform.position + moveVec * dt * 5


	if(input.isKeyPressed(Keys.P)) then
		print("trying to send polygon data to server")
		if(network.isServer()) then
			network.sendPolygons(pols)
		else
			print("we are not server")
		end
	end
end

return script;
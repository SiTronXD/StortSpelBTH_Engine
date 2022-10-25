local script = {}

function script:init()
	print("Cam: " .. self.ID)
	self.texture = resources.addTexture("assets/textures/test_A.png")
end

function script:update()
	uiRenderer.setTexture(self.texture)
	uiRenderer.renderTexture(0, 0, 100, 100)
	debugRenderer.renderCapsule(vector(), vector(), 10, 3, core.LIGHTGRAY)
	local payload = physics.raycast(self.transform.position, self.transform:forward(), 100)
	physics.renderDebugShapes(payload ~= nil)
end

return script;
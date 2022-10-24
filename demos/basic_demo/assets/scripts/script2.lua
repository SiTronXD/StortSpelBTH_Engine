local script = {}

function script:init()
	print("Cam: " .. self.ID)
	self.texture = resources.addTexture("assets/textures/test_A.png")
end

function script:update()
	uiRenderer.setTexture(self.texture)
	uiRenderer.renderTexture(0, 0, 100, 100)
end

return script;
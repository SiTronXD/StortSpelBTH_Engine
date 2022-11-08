local button = {}
button.counter = 0

function button:init()
	self.activeTexture = self.aTexture
	self.aTexture = resources.addTexture("assets/textures/test_A.png", { filterMode = Filters.Nearest })
	self.bTexture = resources.addTexture("assets/textures/test_B.png", { filterMode = Filters.Nearest })
end

function button:update(dt)
	local area = scene.getComponent(self.ID, CompType.UIArea)

	uiRenderer.setTexture(self.activeTexture)
	--uiRenderer.renderTexture(area.position, area.dimension)
	self.activeTexture = self.bTexture

	scene.setComponent(self.ID, CompType.UIArea, area)
end

function button:onHover()
	self.activeTexture = self.aTexture
end

function button:onClick()
	print("Clicked!")
end

return button
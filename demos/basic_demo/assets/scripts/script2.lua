local script = {}

function script:init()
	print("Cam: " .. self.ID)
	self.texture = resources.addTexture("assets/textures/test_A.png", { filterMode = Filters.Nearest })
	--scene.setComponent(self.ID, CompType.Collider, { type = ColliderType.Sphere, isTrigger = true, radius = 1 })
	--scene.setComponent(self.ID, CompType.Rigidbody, {})
end

function script:update()
	--debugRenderer.renderCapsule(vector(), vector(), 10, 3, core.YELLOW)

	local payload = physics.raycast(self.transform.position, self.transform:forward(), 100)
	--physics.renderDebugShapes(payload ~= nil)

	input.setHideCursor(input.isKeyDown(Keys.L))
end

function script:test()
	--print("Test")
end

function script:onTriggerStay(entity)
	print("Hit: " .. entity)
end

return script;
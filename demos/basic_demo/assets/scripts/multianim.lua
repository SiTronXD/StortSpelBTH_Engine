local script = {}

function script:init()
	self.transform.position.x = -30
	self.transform.rotation.x = 90
end

function script:update(dt)
	if (input.isKeyReleased(Keys.ONE)) then
		scene.setAnimation(self.ID, "bendIdle")
	elseif (input.isKeyReleased(Keys.TWO)) then
		scene.setAnimation(self.ID, "fastBend")
	elseif (input.isKeyReleased(Keys.THREE)) then
		scene.setAnimation(self.ID, "dumb")
	end
end

return script
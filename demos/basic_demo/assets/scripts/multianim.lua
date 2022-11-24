local script = {}

function script:init()
	self.transform.position.x = -30
end

function script:update(dt)

	if (input.isKeyReleased(Keys.ONE)) then
		scene.setAnimation(self.ID, "run", "UpperBody")
	elseif (input.isKeyReleased(Keys.TWO)) then
		scene.setAnimation(self.ID, "idle", "UpperBody")
	elseif (input.isKeyReleased(Keys.THREE)) then
		scene.setAnimation(self.ID, "attack", "UpperBody")

	elseif (input.isKeyReleased(Keys.FOUR)) then
		scene.setAnimation(self.ID, "run", "LowerBody")
	elseif (input.isKeyReleased(Keys.FIVE)) then
		scene.setAnimation(self.ID, "idle", "LowerBody")
	elseif (input.isKeyReleased(Keys.SIX)) then
		scene.setAnimation(self.ID, "attack", "LowerBody")

	elseif (input.isKeyReleased(Keys.SEVEN)) then
		local slot = scene.getAnimationSlot(self.ID, "UpperBody")
		scene.setAnimationSlot(self.ID, "UpperBody", slot.animationIndex, slot.timer, 0.1)
	elseif (input.isKeyReleased(Keys.EIGHT)) then
		local slot = scene.getAnimationSlot(self.ID, "UpperBody")
		scene.setAnimationSlot(self.ID, "UpperBody", slot.animationIndex, slot.timer, 3.0)
	elseif (input.isKeyReleased(Keys.NINE)) then
		local slot = scene.getAnimationSlot(self.ID, "UpperBody")
		scene.setAnimationSlot(self.ID, "UpperBody", slot.animationIndex, 0.0, 1.0)
		scene.setAnimationSlot(self.ID, "LowerBody", slot.animationIndex, 0.0, 1.0)
	end

end

return script
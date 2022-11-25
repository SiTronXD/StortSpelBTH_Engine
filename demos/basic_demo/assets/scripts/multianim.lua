local script = {}

function script:init()
	self.transform.position.x = -40
	self.transform.rotation.y = 180
end

function script:update(dt)

	if (input.isKeyReleased(Keys.ONE)) then
		-- Does the same thing as below
		local anim = scene.getComponent(self.ID, CompType.Animation)
		anim[1].animationIndex = 0
		anim[0].timeScale = 0.5
		scene.setComponent(self.ID, CompType.Animation, anim)

		--scene.setAnimation(self.ID, "run", "UpperBody")
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
		slot.timeScale = 0.1
		scene.setAnimationSlot(self.ID, "UpperBody", slot)
	elseif (input.isKeyReleased(Keys.EIGHT)) then
		local slot = scene.getAnimationSlot(self.ID, "UpperBody")
		slot.timeScale = 3.0
		scene.setAnimationSlot(self.ID, "UpperBody", slot)
		scene.setAnimationSlot(self.ID, "LowerBody", slot)
	elseif (input.isKeyReleased(Keys.NINE)) then
		local slot = scene.getAnimationSlot(self.ID, "UpperBody")
		slot.timer = 0.0
		slot.timeScale = 0.1
		scene.setAnimationSlot(self.ID, "UpperBody", slot)
		scene.setAnimationSlot(self.ID, "LowerBody", slot)
	elseif (input.isKeyReleased(Keys.ZERO)) then
		local slot1 = scene.getAnimationSlot(self.ID, "UpperBody")
		local slot2 = scene.getAnimationSlot(self.ID, "LowerBody")
		slot1.timeScale = 1.0
		slot1.timer = 0.0
		slot2.timeScale = 1.0
		slot2.timer = 0.0
		scene.setAnimationSlot(self.ID, "UpperBody", slot1)
		scene.setAnimationSlot(self.ID, "LowerBody", slot2)


	end

end

return script
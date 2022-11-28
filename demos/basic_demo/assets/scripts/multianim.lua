local script = {}

function script:init()
	self.transform.position.x = -40
	self.transform.rotation.y = 180
end

function script:update(dt)

	-- Synced slot blend
	if (input.isKeyReleased(Keys.FOUR) and input.isKeyDown(Keys.SHIFT)) then
		scene.syncedBlendToAnimation(self.ID, "LowerBody", "UpperBody", 0.17)
	elseif (input.isKeyReleased(Keys.FOUR) and input.isKeyDown(Keys.CTRL)) then
		scene.syncedBlendToAnimation(self.ID, "UpperBody", "LowerBody", 0.17)
		
	-- Normal slot blend
	elseif (input.isKeyReleased(Keys.ONE) and input.isKeyDown(Keys.SHIFT)) then
		scene.blendToAnimation(self.ID, "run", "UpperBody", 0.17, 1.0)
	elseif (input.isKeyReleased(Keys.TWO) and input.isKeyDown(Keys.SHIFT)) then
		scene.blendToAnimation(self.ID, "idle", "UpperBody", 0.17, 1.0)
	elseif (input.isKeyReleased(Keys.THREE) and input.isKeyDown(Keys.SHIFT)) then
		scene.blendToAnimation(self.ID, "attack", "UpperBody", 0.17, 1.0)

	elseif (input.isKeyReleased(Keys.ONE) and input.isKeyDown(Keys.CTRL)) then
		scene.blendToAnimation(self.ID, "run", "LowerBody", 0.17, 0.3)
	elseif (input.isKeyReleased(Keys.TWO) and input.isKeyDown(Keys.CTRL)) then
		scene.blendToAnimation(self.ID, "idle", "LowerBody", 0.17, 0.3)
	elseif (input.isKeyReleased(Keys.THREE) and input.isKeyDown(Keys.CTRL)) then
		scene.blendToAnimation(self.ID, "attack", "LowerBody", 0.17, 0.3)
	

	-- Synced skeleton blend
	elseif (input.isKeyReleased(Keys.ONE) and input.isKeyDown(Keys.SPACE)) then
		scene.syncedBlendToAnimation(self.ID, "LowerBody", "", 0.17)
	elseif (input.isKeyReleased(Keys.TWO) and input.isKeyDown(Keys.SPACE)) then
		scene.syncedBlendToAnimation(self.ID, "UpperBody", "", 0.17)
	
	-- Set timeScale
	elseif (input.isKeyReleased(Keys.FOUR) and input.isKeyDown(Keys.SPACE)) then
		scene.setAnimationTimeScale(self.ID, 0.3, "UpperBody")
	elseif (input.isKeyReleased(Keys.FIVE) and input.isKeyDown(Keys.SPACE)) then
		scene.setAnimationTimeScale(self.ID, 3.0, "UpperBody")
	elseif (input.isKeyReleased(Keys.SIX) and input.isKeyDown(Keys.SPACE)) then
		scene.setAnimationTimeScale(self.ID, 1.0, "UpperBody")


	-- Normal skeleton blend
	elseif (input.isKeyReleased(Keys.ONE)) then
		scene.blendToAnimation(self.ID, "run", "", 0.17, 0.3)
	elseif (input.isKeyReleased(Keys.TWO)) then
		scene.blendToAnimation(self.ID, "idle", "", 0.17, 0.3)
	elseif (input.isKeyReleased(Keys.THREE)) then
		scene.blendToAnimation(self.ID, "attack", "", 0.17, 0.3)

	elseif (input.isKeyReleased(Keys.G)) then
		local status = scene.getAnimationStatus(self.ID, "UpperBody")
		print(status.animationName)
		print(status.timer)
		print(status.timeScale)
		print(status.endTime)
		print(status.finishedCycle)
	
	elseif (input.isKeyReleased(Keys.SEVEN)) then
		scene.setAnimation(self.ID, "idle", "", 0.5);
	end



	--if (input.isKeyReleased(Keys.ONE)) then
	--	scene.setAnimation(self.ID, "run", "UpperBody")
	--elseif (input.isKeyReleased(Keys.TWO)) then
	--	scene.setAnimation(self.ID, "idle", "UpperBody")
	--elseif (input.isKeyReleased(Keys.THREE)) then
	--	scene.setAnimation(self.ID, "attack", "UpperBody")
	--
	--elseif (input.isKeyReleased(Keys.FOUR)) then
	--	scene.setAnimation(self.ID, "run", "LowerBody")
	--elseif (input.isKeyReleased(Keys.FIVE)) then
	--	scene.setAnimation(self.ID, "idle", "LowerBody")
	--elseif (input.isKeyReleased(Keys.SIX)) then
	--	scene.setAnimation(self.ID, "attack", "LowerBody")
	--
	--elseif (input.isKeyReleased(Keys.SEVEN)) then
	--	local slot = scene.getAnimationSlot(self.ID, "UpperBody")
	--	scene.setAnimationSlot(self.ID, "UpperBody", slot.animationIndex, slot.timer, 0.1)
	--elseif (input.isKeyReleased(Keys.EIGHT)) then
	--	local slot = scene.getAnimationSlot(self.ID, "UpperBody")
	--	scene.setAnimationSlot(self.ID, "UpperBody", slot.animationIndex, slot.timer, 3.0)
	--elseif (input.isKeyReleased(Keys.NINE)) then
	--	local slot = scene.getAnimationSlot(self.ID, "UpperBody")
	--	scene.setAnimationSlot(self.ID, "UpperBody", slot.animationIndex, 0.0, 1.0)
	--	scene.setAnimationSlot(self.ID, "LowerBody", slot.animationIndex, 0.0, 1.0)
	--end

end

return script
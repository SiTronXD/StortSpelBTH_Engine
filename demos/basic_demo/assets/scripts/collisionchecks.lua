local script = {}

function script:init()
	print(self.ID .. " init")
	scene.setComponent(self.ID, CompType.Mesh, 0)
	scene.setComponent(self.ID, CompType.Collider, { type = ColliderType.Sphere, radius = 2, isTrigger = true})
	scene.setComponent(self.ID, CompType.Rigidbody, {})
	self.transform.position = vector(0, 10, 0)
end

function script:onTriggerEnter(e)
	print("Enter: " .. self.ID)
end

function script:onTriggerStay(e)
	print("Stay: " .. self.ID)
end

function script:onTriggerExit(e)
	print("Exit: " .. self.ID)
end

return script
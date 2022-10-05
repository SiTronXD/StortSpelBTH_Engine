local system = {}
system.counter = 1

function system:update(dt)
	local path = "assets/scripts/"
	local view = { path .. "script.lua", CompType.Mesh, CompType.Transform }
	local dt = dt
	scene.iterateView(self, view, self.viewFunc)

	self.counter = self.counter - 1
	return self.counter < 1
end

function system:viewFunc(script, mesh, transform)
	print(script.speed)
	print(mesh.meshID)
	print(transform.position)
end

return system;
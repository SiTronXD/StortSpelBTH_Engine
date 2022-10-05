local cam = scene.createEntity()
scene.setComponent(cam, CompType.Camera)
scene.setMainCamera(cam)

local e = scene.createEntity()
scene.setComponent(e, CompType.Mesh)

local transform = {
	position = vector(3, 0, 3),
	rotation = vector(0, 0, 0),
	scale = vector.fill(1)
}
scene.setComponent(e, CompType.Transform, transform)
scene.setComponent(e, CompType.Behaviour, "assets/scripts/script.lua")
scene.createSystem("assets/scripts/system.lua")

local ghost = resources.addMesh("assets/models/ghost.obj")
print(ghost)

local cam = scene.createEntity()
scene.setComponent(cam, CompType.Camera, { fov = 90 })
scene.setMainCamera(cam)

--local p = scene.createPrefab("assets/scripts/prefabs/prefab.lua")

scene.setComponent(cam, CompType.Script, "assets/scripts/script2.lua")
scene.getComponent(cam, CompType.Script).playerID = p

local e = scene.createEntity()
scene.setComponent(e, CompType.Script, "assets/scripts/collisionchecks.lua")

--[[local prefab = {
	Transform = {
		position = vector(3, 0, 5),
		rotation = vector(0, 45, -90),
		scale = vector.fill(1)
	},
	Mesh = 0,
	Script = "assets/scripts/script.lua"
}
scene.createPrefab(prefab)

prefab.Transform = {
	position = vector(-3, 0, 5),
	rotation = vector(0, -45, -90),
	scale = vector.fill(1)
}
scene.createPrefab(prefab)

scene.createSystem("assets/scripts/system.lua")]]--

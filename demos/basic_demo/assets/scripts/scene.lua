local ghost = resources.addMesh("assets/models/ghost.obj")
print(ghost)

local cam = scene.createEntity()
scene.setComponent(cam, CompType.Camera, { fov = 90 })
scene.setMainCamera(cam)

--local p = scene.createPrefab("assets/scripts/prefabs/prefab.lua")

scene.setComponent(cam, CompType.Script, "assets/scripts/script2.lua")
scene.getComponent(cam, CompType.Script).playerID = p

local multiAnim = resources.addAnimations({ "assets/models/char/cRun.fbx","assets/models/char/cIdle.fbx", "assets/models/char/cAttack2.fbx" })
resources.mapAnimations(multiAnim, {"run", "idle", "attack"})
resources.createAnimationSlot(multiAnim, "LowerBody", "mixamorig:Hips");
resources.createAnimationSlot(multiAnim, "UpperBody", "mixamorig:Spine1");

local a = scene.createEntity()
scene.setComponent(a, CompType.Mesh, multiAnim)
scene.setComponent(a, CompType.Animation)
scene.setComponent(a, CompType.Script, "assets/scripts/multiAnim.lua")

local e = scene.createEntity()
scene.setComponent(e, CompType.Script, "assets/scripts/collisionchecks.lua")

local ui = scene.createEntity()
scene.setComponent(ui, CompType.UIArea, { position = vector(0, 0, 0), dimension = vector(250, 100, 0) } )
scene.setComponent(ui, CompType.Script, "assets/scripts/button.lua")

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

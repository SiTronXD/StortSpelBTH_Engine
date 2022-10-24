local ghost = resources.addMesh("assets/models/ghost.obj")
print(ghost)

--local cam = scene.createEntity()
--scene.setComponent(cam, CompType.Camera)
--scene.setMainCamera(cam)

local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
scene.setMainCamera(cam)

local player = scene.createEntity()
scene.setComponent(player, CompType.Mesh, ghost)
scene.setComponent(player, CompType.Script, "scripts/Movement.lua")
scene.getComponent(cam, CompType.Script).playerID = player
scene.getComponent(player, CompType.Script).camID = cam
network.sendPlayer(player)

scene.createSystem("scripts/system.lua")
local cam = scene.createEntity()
scene.setComponent(cam, CompType.Camera)
scene.setMainCamera(cam)

local e = scene.createEntity()
scene.setComponent(e, CompType.Mesh)

local transform = scene.getComponent(e, CompType.Transform)
transform.position = vector(0, 0, 3)
transform.rotation = vector(-90, 0, 0)
scene.setComponent(e, CompType.Transform, transform)
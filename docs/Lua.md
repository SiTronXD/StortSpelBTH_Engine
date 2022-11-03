# Lua Documentation - Vengine
This file contains everything lua related that has been implemented in to Vengine.

## 1 Table of contents
* [Metatables and Globals](#Metatables-and-Globals)
	* [Core](#Core)
	* [Vector](#Vector)
* [Exposed Systems:](#Exposed-Systems)
	* [Scene](#Scene)
		* [Components](#Components)
	* [Input](#Input)
	* [Resource Manager](#Resource-Manager)
	* [Network](#Network)
	* [Physics](#Physics)
	* [UIRenderer](#UIRenderer)
	* [DebugRenderer](#DebugRenderer)
* [Scene Files](#Scene-Files)
* [Prefabs](#Prefabs)
* [Script Component](#Script-Component)
* [Lua Systems](#Lua-Systems)

## Metatables and Globals
Metatables within Lua is a way to share functionality between tables. The concept could be compared to normal classes in other program languages, such as C++. When using a metatable, operators and other common functionality within tables can be overwritten such as addition, subtraction, conversion to string, constructors, control of the addition of entries to the table etc.

### Core
The core global contains common functionality that is useful in a variety of situations. Currently it's very small but later will be added upon after demand.

#### Colors
Core contains some default colors to be used in other global functions. It uses the [vector](#Vector) global. List of currently added colors:
~~~ Lua
-- Some default colors
core.RED = vector(1, 0, 0)
core.GREEN = vector(0, 1, 0)
core.BLUE = vector(0, 0, 1)
core.YELLOW = vector(1, 1, 0)
core.MAGENTA = vector(1, 0, 1)
core.CYAN = vector(0, 1, 1)
core.BLACK = vector(0, 0, 0)
core.WHITE = vector(1, 1, 1)
core.GRAY = vector(0.5, 0.5, 0.5)
core.LIGHTGRAY = vector(0.8, 0.8, 0.8)
~~~

### Vector
The vector lua table is one of the most used metatables in the lua environment. It is a table that contains a x, y and z value and functions and overloading of functions to match the vector classes used in C++. It's also used for a variation of components such as Transform.

Note: In the meantime only 3D vectors have been implemented and if there aren't any reason to, it will also be used as a 2D vector (but with an extra z-value).

Notable functions:
~~~ Lua
vector(x, y, z) -- Constructor
vector.new(x, y, z) -- Also constructor
vector.fill(v) -- Returns a vector where all values are "v" (vector(v, v, v))
vector.random(scalar) --Makes this vector random between -1 and 1 times scalar
vector.randomPositive(scalar) --Makes this vector random between 0 and 1 times scalar
vector:length() -- Returns length of vector
vector:cross(v) -- Return the cross product between itself and vector v
vector:normalize() -- Normalizes the vector
vector:rotate(x, y, z) -- Rotates the vector based on values
vector:rotateX(xrot) -- Rotates the vector on the X-axis
vector:rotateY(yrot) -- Rotates the vector on the Y-axis
vector:rotateZ(zrot) -- Rotates the vector on the Z-axis
~~~

## Exposed Systems
Probably where most information will be written and expanded upon, where C++ classes/systems that have been exposed down to lua will have their functions explained with parameter lists.

Names of the system globals in lua:
* scene
* input
* resources
* network
* physics
* uiRenderer
* debugRenderer

## Scene
The scene global that has been created in the lua environment is the main interface of the ECS, similarly to the C++ environment. It has multiple functions that is the same as in C++ and some extra functionality.

With the scene global comes 2 other globals that is related to the components and systems. These are called *CompType* and *SystemType* and their values are used as the replacement of template types within C++ since lua doesn't support them. Examples of how this is used can be seen in [createSystem](#createSystem), [hasComponent](#hasComponent), [getComponent](#getComponent), [setComponent](#setComponent) and more.

### Functions
List of functions related to the *scene* global.

#### createSystem
This function is used to create a system within the current scene. There are options for both [lua systems](#Lua-Systems) and C++ systems (within Vengine) is available.
~~~ Lua
scene.createSystem(string : lua_path) -- Lua systems
scene.createSystem(int : system_type, { args }) -- C++ systems
~~~
The global *SystemType* is useful when creating a C++ system with this class. Example:
~~~ Lua
-- Creates an instance of the example system (doesn't exist)
scene.createSystem(SystemType.ExampleSystem, exampleInt, exampleString) 
~~~

#### setScene
Switches to a new scene with a [lua scene file](#Scene-Files) as the initializer. Currently this only creates an instance of the Scene class and not any other child classes that can be done in C++. Could be changed in the future.
~~~ Lua
scene.setScene(string : lua_path) -- Lua scene file
~~~

#### iterateView
Function often used in combination of a [lua system](#Lua-System) that takes a table as the view of the entities with the rquired components. The second is the function used on each entity returned from the scene. It also takes a third argument in for additional data, which is mostly used in the [lua systems](#Lua-System).

~~~ Lua
scene.iterateView(table : view, func : iterate_function,
table : additional_data) -- Not required, will be first param in the function

-- Example
local view = {CompType.Transform, CompType.Mesh, "script.lua"}

function iterateFunction(transform, mesh, script)
	print(transform.position)
	print(mesh.meshID)
	print(script.exampleInt)
end

scene.iterateView(view, iterateFunction)
~~~

#### createPrefab
This functions is only available in Lua and uses a [prefab](#Prefabs) table to create an entity with desired components with desired values within them. This can be done via a table directly or a lua file that returns a prefab. The function returns an entity ID.
~~~ Lua
scene.createPrefab(string : lua_path) -- Lua file that returns a prefab table
scene.createPrefab(table : prefab) -- Prefab table

--Example prefab
local prefab = {
	Transform = {
		position = vector(0, 0, 3),
		rotation = vector(0, 90, 0),
		scale = vector.fill(1)
	},
	Mesh = 0, -- Could be a table in the future if there are more members
	Script = "script.lua"
}
local entity = scene.createPrefab(prefab) -- creates entity with the components
~~~

#### getMainCamera
Returns entity ID of the main camera within the scene.
~~~ Lua
local cam = scene.getMainCamera()
~~~

#### setMainCamera
Sets the main camera in the scene. Entity sent in must have a camera component.

~~~ Lua
scene.setMainCamera(int : entity) -- Must have camera component before
~~~

#### getEntityCount
Returns the entity count within the scene.
~~~ Lua
local numEntities = scene.getEntityCount() -- Returns int
~~~

#### entityValid
Checks if given entity ID actually exists in the scene.
~~~ Lua
scene.entityValid(int : entity) -- Returns true or false
~~~

#### createEntity
Creates an entity in the scene and returns the ID.
~~~ Lua
local entity = scene.createEntity() -- Returns entity ID
~~~

#### removeEntity
Removes an entity in the scene. Returns boolean that decribe if it existed before.
~~~ Lua
scene.removeEntity(int : entity) -- Returns true or false
~~~

#### hasComponent
Checks if an entity has a component assigned to it. The global *CompType* is useful here for the component type number.
~~~ Lua
scene.hasComponent(int : entity, int : component_type) -- Returns true or false

-- Example: check if entity has a mesh
local bool = scene.hasComponent(entity, CompType.Mesh)
~~~

#### getComponent
Gets a component from a given entity. This function returns it's reprensation of the component (table) or nil if the entity doesn't have the given component type. The global *CompType* is useful here for the component type number.
~~~ Lua
scene.getComponent(int : entity, int : component_type) -- Returns component

-- Example: get mesh component and print the meshID
if (scene.hasComponent(entity, CompType.Mesh)) then
	local mesh = scene.getComponent(entity, CompType.Mesh)
	print(mesh.meshID)
end
~~~

#### setComponent
Set component values or create component to a given entity. The global *CompType* is useful here for the component type number.
~~~ Lua
scene.setComponent(int : entity, int : component_type, { args })

-- Example: create a mesh component and script component
scene.setComponent(entity, CompType.Mesh, "test.obj") -- Use ID or string
scene.setComponent(entity, CompType.Script, "script.lua")
~~~

#### removeComponent
Removes a component to a given entity. The global *CompType* is useful here for the component type number.
~~~ Lua
scene.removeComponent(int : entity, int : component_type)

-- Example: remove mesh component
scene.removeComponent(entity, CompType.Mesh)
~~~

#### setActive
Sets an enitity active if it wasn't before in the scene.
~~~ Lua
scene.setActive(int : entity)
~~~

#### setInactive
Sets an enitity inactive if it wasn't before in the scene.
~~~ Lua
scene.setInactive(int : entity)
~~~

#### isActive
Checks status of an entity. Returns bool describing it's active status
~~~ Lua
scene.isActive(int : entity) -- Returns bool
~~~

### Components
The amount of components supported grows with the engine. This section goes over the member variables that the components have and how to use them in combination with the [scene](#Scene) global and [prefabs](#Prefabs) These are the ones currently supported in lua:
* [Transform](#Transform)
* [MeshComponent](#MeshComponent)
* [Script](#Script)
* [Camera](#Camera)
* [Collider](#Collider)
* [Rigidbody](#Rigidbody)
* [Animation](#Animation)
* AudioListener (not completely done)
* AudioSource (not completely done)

#### Transform
The transform component consists of three [vectors](#Vector) that describe the position, rotation and scale of an entity in the scene.
~~~ Lua
-- Members (also how it's defined in prefabs)
local Transform = {
	vector : position,
	vector : rotaiton,
	vector : scale
}

-- Member functions
transform:right() -- get local right vector
transform:up() -- get local up vector
transform:forward() -- get local forward vector

--Example of use
local t = scene.getComponent(CompType.Transform, e)
t.position = t.position + vector(1, 0, 1)
scene.setComponent(CompType.Transform, e, t)
~~~

#### MeshComponent
The mesh component for now only consists of an integer that is the mesh ID in the [Resource manager](#ResourceManager). In some instances strings can be used for creation
~~~ Lua
-- Members (also how it's defined in prefabs)
local Mesh = int : mesh_ID
local Mesh = string : mesh_path -- For creation strings can be used
local Mesh = { -- When getting from scene.getComponent
	int : meshID
}

--Example of use
local mesh = scene.getComponent(e, CompType.Mesh)
mesh.meshID = resources.addMesh("test.obj")
scene.setComponent(e, CompType.Mesh, mesh)
~~~

#### Script
The script component is used for creating scriptable components with updating every frame. More information in [Script](#Script-Component).
~~~ Lua
-- Members (also how it's defined in prefabs)
local Script = string : path
local Script = { -- When getting from scene.getComponent
	int : ID,
	string : path,
	Transform : transform,
	additional members...
}

--Example of use
local script = scene.getComponent(e, CompType.Script) -- Lua table
print(script.ID)
scene.setComponent(e, CompType.Script, "script.lua")
~~~

#### Camera
The camera component for now only consists of an float for the FOV (field of view)
~~~ Lua
-- Members (also how it's defined in prefabs)
local Camera = float : fov
local Camera = { -- When getting from scene.getComponent
	float : fov
}

--Example of use
local cam = scene.getComponent(e, CompType.Camera)
cam.fov = 90
scene.setComponent(e, CompType.Camera, cam)
~~~~~~

#### Collider
The collider component is used for physics and can be combined with [rigidbody](#Rigidbody) for it to be dynamic. Only using a collider component will result with a kinematic physicsbody.
~~~ Lua
-- Members (also how it's defined in prefabs)
local Collider = {
	int : type, -- ColliderType global useful here
	bool : isTrigger,
	vector : offset, -- Offset from origin
	float : radius, -- Only needed/recieved for spheres and capsules
	float : height, -- Only needed/recieved for capsules
	vector : extents -- Only needed/recieved for boxes
}

--Example of use
local col = scene.getComponent(e, CompType.Collider)
col.type = ColliderType.Sphere
col.radius = 2.5
col.isTrigger = false
col.offset = vector(0, 5, 0)
scene.setComponent(e, CompType.Collider, col)
~~~

#### Rigidbody
The rigidbody component is used to make a physics object dynamic in the scene. Collisions and acceleration is supported. Must be used in combination with [collider component](#Collider), will not create object in [physics engine](#Physics) otherwise.
~~~ Lua
-- Members (also how it's defined in prefabs)
local RigidBody = {
	float : mass,
	float : gravityMult,
	float : friction,
	vector : posFactor,
	vector : rotFactor,
	vector : acceleration,
	vector : velocity
}

--Example of use
local rb = scene.getComponent(e, CompType.Rigidbody)
rb.velocity = rb.velocity + vector(0, 10, 0)
scene.setComponent(e, CompType.Rigidbody, rb)
~~~

#### Animation
The animation component is used to apply an animation on an enity with a [mesh](#MeshComponent). Currently only the timer and playback speed can be altered, but more will be added in the future. Currently only the first animation in the mesh will be played and can't be switched (will be fixed).
~~~ Lua
-- Members (also how it's defined in prefabs)
local Animation = {
	float : timer,
	float : timeScale
}

--Example of use
local anim = scene.getComponent(e, CompType.Animation)
anim.timeScale = -2 -- Backwards at double speed
scene.setComponent(e, CompType.Animation, anim)
~~~

## Input
The input global that has been created in the lua environment is the main interface of the keyboard and mouse inputs.

With the input global comes 2 other globals that is related to the keyboard keys and mouse buttons. These are called *Keys* and *Mouse* and their values are used as the replacement of the enums used within C++ since lua doesn't support it the same way. Examples of how this is used can be seen in most functions within the input global.

### Functions
List of functions related to the *input* global.

#### isKeyDown
Checks if given key is currently pressed down. The global *Keys* is useful here for the key ID number.
~~~ Lua
input.isKeyDown(int : key_id) -- Returns true or false

-- Example: check if the key R is currently down
local bool = input.isKeyDown(Keys.R)
~~~

#### isKeyUp
Checks if given key is currently not pressed. The global *Keys* is useful here for the key ID number.
~~~ Lua
input.isKeyUp(int : key_id) -- Returns true or false

-- Example: check if the key R is currently up
local bool = input.isKeyUp(Keys.R)
~~~

#### isKeyPressed
Checks if given key was just pressed down. The global *Keys* is useful here for the key ID number.
~~~ Lua
input.isKeyPressed(int : key_id) -- Returns true or false

-- Example: check if the key R was just pressed
local bool = input.isKeyPressed(Keys.R)
~~~

#### isKeyReleased
~~~ Lua
input.isKeyReleased(int : key_id) -- Returns true or false

-- Example: check if the key R was just released
local bool = input.isKeyReleased(Keys.R)
~~~

#### isMouseButtonDown
Checks if given mouse button is currently pressed down. The global *Mouse* is useful here for the mouse ID number.
~~~ Lua
input.isMouseButtonDown(int : mouse_id) -- Returns true or false

-- Example: check if the mouse button LEFT is currently down
local bool = input.isMouseButtonDown(Mouse.LEFT)
~~~

#### isMouseButtonUp
Checks if given mouse button is currently not pressed. The global *Mouse* is useful here for the mouse ID number.
~~~ Lua
input.isMouseButtonUp(int : mouse_id) -- Returns true or false

-- Example: check if the mouse button LEFT is currently up
local bool = input.isMouseButtonUp(Mouse.LEFT)
~~~

#### isMouseButtonPressed
Checks if given mouse button was just pressed down. The global *Mouse* is useful here for the mouse ID number.
~~~ Lua
input.isMouseButtonPressed(int : mouse_id) -- Returns true or false

-- Example: check if the mouse button LEFT was just pressed
local bool = input.isMouseButtonPressed(Mouse.LEFT)
~~~

#### isMouseButtonReleased
Checks if given mouse button was just released. The global *Mouse* is useful here for the mouse ID number.
~~~ Lua
input.isMouseButtonReleased(int : mouse_id) -- Returns true or false

-- Example: check if the mouse button LEFT was just released
local bool = input.isMouseButtonReleased(Mouse.LEFT)
~~~

#### getMousePosition
Function used to get the mouse position on the screen. This returns a [Vector](#Vector) that is treated as a 2D vector.
~~~ Lua
input.getMousePosition() -- Returns vector
~~~

#### getMouseDelta
Function used to get the mouse delta on the screen. This returns a [Vector](#Vector) that is treated as a 2D vector. The vector is created from the difference of the current position and the postion from the frame before.
~~~ Lua
input.getMouseDelta() -- Returns vector
~~~

#### getScrollWheelDelta
Function used to get the scroll wheel delta. The function returns an integer describing how much the scrollwheel is scrolling.
~~~ Lua
input.getScrollWheelDelta() -- Returns int
~~~

#### setCursorPosition
Function used to manually set the cursor's position on the screen. Argument is a 2D [Vector](#Vector) where vector(0, 0) is the top left of the screen.
~~~ Lua
input.setCursorPosition(vector : screen_pos)
~~~

#### setHideCursor
Functions that hides/shows the cursor depending on bool sent in. When being hidden, the mouse can't leave the screen and [getMouseDelta](#getMouseDelta) function still works as expected.
~~~ Lua
input.setHideCursor(bool : hide)
~~~

## Resource Manager
The resources global that has been created in the lua environment is the main interface of the resource manager in C++. It is used to load in a variety of assets from disk, such as meshes and textures.

With the resources global comes another global table that is related to Filter modes to textures. These value can be used in a table when [adding a texture](#addTexture) to the resource manager.
~~~ Lua
-- Filters
Filters.Nearest
Filters.Linear
Filters.CubicEXT -- Not available currently (warnings)
Filters.CubicIMG -- Not available currently (warnings)
~~~

### Functions
List of functions related to the *resources* global.

#### addMesh
This functions loads and creates a mesh to be used in the engine. It returns an ID of the mesh that can be used to reference it in different part of the engine such as in components. If the path given already has been loaded, the ID is returned immediatly. If the mesh couldn't be created a default mesh ID is returned. An optional string can also be sent in as the texture path.
~~~ Lua
resources.addMesh(string : mesh_path) -- Returns the mesh ID
resources.addMesh(string : mesh_path, string : textures_path) -- optional
~~~

#### addTexture
This functions loads and creates a texture to be used in the engine. It returns an ID of the texture that can be used to reference it in different part of the engine such as in components. If the path given already has been loaded, the ID is returned immediatly. If the texture couldn't be created a default texture ID is returned.
Additionally a table containing sampling information can be sent as a second argument.
~~~ Lua
resources.addTexture(string : texture_path) -- Returns the texture ID
resources.addTexture(string : texture_path, table : sampler_settings) -- optional

-- Example
local setttings = {}
settings.filterMode = Filters.Linear
local textureID = resources.addTexture("test.png", settings)
~~~

#### addAudio
Loads audio file that can be used in the engine. Returns ID if the path sent is successfully loaded the audiofile. Otherwise the return is nil.
~~~ Lua
resources.addAudio(string : audio_path) -- Returns the audio ID (nil if not found)
~~~

## Network
The network global that has been created in the lua enviroment is the main interface of the network handler in C++. 

### Functions
List of functions related to the *network* global.

#### sendPolygons
Sends in a table of vectors to the server if the current client has a server. The polygons is then used for the AI to navigate the map.
~~~ Lua
local points = {
	vector(-1,0,-1),
	vector(1,0,-1),
	vector(-1,0,1),
}
network.sendPolygons(pols)
~~~

#### isServer
Return a bool if the current user is a host to a server or not
~~~ Lua
local bool = network.isServer()
~~~

#### sendTCPData
Send data to server through the TCP protocol.	
~~~ Lua
sendTCPData(int : callType, table.int : data, table.float : data)
~~~
For more info check NetworkEnumAndDefines.h

#### sendUDPData
Sends data to server through the UDP protocol.
In this specific project, you are only able to send your player position and rotation.
Arguments:
	Position: vector with players position
	Rotation: vector with players rotation
~~~ Lua
sendUDPData(vector : player_position, vector : player_rotation)
~~~

#### getNetworkData
In progress...

## Physics
The physics global that has been created for the lua environment is the main interface for simple physics functions such as [raycast](#raycast) and [contactTest](#contactTest).

### Functions
List of functions related to the *physics* global.

#### renderDebugShapes
This function takes a bool parameter to decide if the physics engine should render debug primitives for the colliders within the current scene.
~~~ Lua
physics.renderDebugShapes(bool : renderDebug)
~~~

#### raycast
Define a ray by a position and direction and cast it to the physics scene. If something was hit the functions returns a table containing entity and some other information. A distance value can also be sent in as an argument. The default value is 100.
~~~ Lua
physics.raycast(vector : pos, vector : dir, float dist) -- dist is optional

-- Returning table (if hit)
local payload = {
	int : entity,
	vector : hitPosition,
	vector : hitNormal
}

-- Example
local payload = physics.raycast(vector(0, 0, 0), vector (0, 0, 1), 100)
if(payload) then
	print("Hit entity: " .. payload.entity .. " at pos: " .. payload.position)
end

~~~

#### testContact
Send in a [Collider](#Collider) as the collision shape and position and rotation. Rotation is however optional. The function returns a table as an array of all entities hit with the collision shape.
~~~ Lua
physics.testContact(collider : shape, vector : pos, vector : rot)

-- Example
local list = physics.testContact(col, vector(0, 0, 0), vector(90, 0, 0))
for i = 1, #list do
	print("Hit entity: " .. list[i])
end
~~~

## UIRenderer
The uiRenderer global that has been created for the lua environment is the main interface to render textures to the screen as UI.

### Functions
List of functions related to the *uiRenderer* global.

#### setTexture
Sets the active texture resource to be rendered when calling [renderTexture](#renderTexture). It can be used with a texture ID or the path to the texture.
~~~ Lua
uiRenderer.setTexture(int : textureID)
uiRenderer.setTexture(string : texture_path)
~~~

#### renderTexture
Renders the active texture to the screen. [setTexture](#setTexture) should be called before. Takes a 2D position and width and height as the argument.
~~~ Lua
uiRenderer.renderTexture(float : x, float : y, float : width, float : height)

--Example use
uiRenderer.setTexture("test.png")
uiRenderer.renderTexture(0, 0, 100, 100) -- 100x100 image in the middle of the screen
~~~

## DebugRenderer
The debugRenderer global that has been created for the lua environment is the main interface to render debug lines and wireframe objects. This can be used in combination of [Physics](#Physics) for colliders and more.

### Functions
List of functions related to the *debugRenderer* global.

#### renderLine
Renders a line of a designated color.
~~~ Lua
debugRenderer.renderLine(
vector : pos0,
vector : pos1,
vector : color)
~~~

#### renderSphere
Renders a sphere of a designated position, radius and color
~~~ Lua
debugRenderer.renderSphere(
vector : position,
float : radius,
vector : color)
~~~

#### renderBox
Renders a box of a designated position, rotation, size and color
~~~ Lua
debugRenderer.renderBox(
vector : position,
vector : rotation,
vector : size,
vector : color)
~~~

#### renderCapsule
Renders a capsule of a designated position, rotation, height, radius and color
~~~ Lua
debugRenderer.renderCapsule(
vector : position,
vector : rotation,
float : height,
float : radius,
vector : color)
~~~

#### renderSkeleton
Renders a skeleton on an entity of a designated color.
~~~ Lua
debugRenderer.renderSkeleton(
int : entityID,
vector : color)
~~~

## Scene Files
Scene files are lua scripts that describe the initilization of a scene and is mostly used in that context. What is done in the lua file is up to the user, but creating entities and setting components within the [Scene](#Scene). Currently these are called when creating a new scene in C++ and via the [setScene](#setScene) function in lua.

~~~ Lua
-- Example scene
print("Init scene.lua")

-- Loading resources
local test = resources.addMesh("test.obj")

-- Setting the camera
local cam = scene.createEntity()
scene.setComponent(cam, CompType.Camera)
scene.setMainCamera(cam)

-- Creating additional entities in the scene
local p = scene.createPrefab("prefab.lua")

local prefab = {
	Transform = {
		position = vector(3, 0, 5),
		rotation = vector(0, 45, -90),
		scale = vector.fill(1)
	},
	Mesh = test,
	Script = "script.lua"
}
scene.createPrefab(prefab)

-- Creating a system
scene.createSystem("exampleSystem.lua")
~~~

## Prefabs
Prefabs are lua tables that contain component information as the elements. This makes the process of creating entities easier since components and their start data can be defined in one variable and later be called with one function call. This is done with the [createPrefab](#createPrefab) function in the [scene](#Scene).
~~~ Lua
-- Difference of using prefabs vs manual calls

-- Not using prefabs
local entity = scene.createEntity()
transform = { 
	position = vector(3, 0, 5),
	rotation = vector(0, 45, -90),
	scale = vector.fill(1) 
}
scene.setComponent(entity, CompType.Transform, transform)
scene.setComponent(entity, CompType.Mesh, "test.obj")
scene.setComponent(entity, CompType.Script, "script.lua")

-- Using prefabs
local prefab = { 
	Transform = { 
		position = vector(3, 0, 5),
		rotation = vector(0, 45, -90),
		scale = vector.fill(1) 
	}, 
	Mesh = "test.obj",
	Script = "script.lua"
} 
scene.createPrefab(prefab) -- Prefab could also be from lua file
~~~
The advantage of prefabs is that they also can be reused for multiple entities and stored in their own lua file. 

The values the elements have in the prefab table depends on the component and examples of it are shown below. The name of the prefab table elements and their elements need to be 100% correct for it to work in C++.

### Supported Components
~~~ Lua
-- Transform: table containing vectors
Transform = {
	position = vector()
	rotation = vector()
	scale = vector()
}

-- Mesh: int for meshID or string as asset path
Mesh = int : meshID
Mesh = string : mesh_path

-- Script: string for lua path
Script = string : lua_path

-- Camera: float for Field of View
Camera = float : fov
~~~

## Script Component
Script components are used as a way to create custom functionality to entities without having to create a component struct and after a system only used for that component. Scripts are also more customizable and can more easily be modified during runtime due to it being defined as a lua file.

Script components are applied similarly to other components within the [scene](#Scene) with it's only argument being a file path to a lua script. The files are constructed via creating a local table with data and functions and after returning it at the end.

~~~ Lua
local script = {} -- Script table

-- Some member variables
script.test = 1
script.name = "Script"

-- Some member functions
function script.test()
end

return script -- Return script table
~~~
When using the script, some function names will be called from C++ if they are defined. These are used for the functionality that is required in a script component.
~~~ Lua
script:init() -- Called after component has been created
script:update(dt) -- Called every new frame where dt is the delta time.
script:onCollisionEnter(e) -- Called when collision with another entity has started
script:onCollisionStay(e) -- Called when colliding with another entity
script:onCollisionExit(e) -- Called when collision with another entity has ended
script:onTriggerEnter(e) -- Called when triggering with another entity has started
script:onTriggerStay(e) -- Called when trigger colliding with another entity
script:onTriggerExit(e) -- Called when triggering with another entity has ended
~~~
The script table created and used in these functions also have some extra member elements that has been provided by the C++ side.
~~~ Lua
script.ID -- Entity ID of the entity attached
script.path -- Path to the script file
script.transform -- Own transform component that is updated after function calls from C++
~~~

## Lua Systems
Systems defined in the lua environment is done via a file. Similarly to [script components](#Script-Component) a table is created and returned. The function *update* also needs to be defined since it's going to get called every frame.
~~~ Lua
local system = {}

function system:update(dt) -- Called every frame from C++
	return false -- Returns true or false. 
	-- true: the system will be destroyed.
	-- false: nothing will happen.
end

return system
~~~
That is basically it for lua systems. In addtion to this the [iterateView](#iterateView) function in the [scene global](#Scene) is often used in systems.
~~~ Lua
-- Example system
local system = {}

function system:update(dt)
	local view = { "script.lua", CompType.Mesh, CompType.Transform }
	self.dt = dt
	scene.iterateView(view, self.viewFunc, self)

	return false
end

function system:viewFunc(script, mesh, transform)
	print(script.ID)
	print(mesh.meshID)
	print(transform.position)
	print(self.dt)
end

return system;
~~~

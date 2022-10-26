# Lua Documentation - Vengine-Server
This document contains things that are different from lua in Vengine in lua Vengine-Server 

## 1 Table of contents
* [Removed functions](#Removed)
* [Added](#Added)

## Removed
### Globals
  Globals that have disepeared are network and resourcehandler. You are not able to retrive any information about the network or able to load any data from the resource handler.
  
### Scene
  Scene has some function taken away and some added here are those that have been removed.
  ~~~ Lua
  setScene
  getMainCamera
  setMainCamera
  ~~~
  
## Added
  ### getPlayerCount
  getPlayerCount returns how many players are playing on the server currently.
  ~~~Lua
  local nrOfPlayers = scene.getPlayerCount()
  ~~~
  ### getPlayer
  getPlayer return the entity id that one of the player has.
  ~~~Lua
  for i = 0, getPlayerCount() then
    local playerID = scene.getPlayer(i)
  end
  ~~~
  
  ### addEvent
  Send a addEvent call to the server.
  The int:s in the event should be called with individually while the float data should be called in a table.
  ~~~Lua
  local entData = { 0,0,70,0,0,90 }
  scene.addEvent(1, 2, entData)
  scene.addEvent(4, 4)
  ~~~
  for more info check NetworkEnumAndDefines.h

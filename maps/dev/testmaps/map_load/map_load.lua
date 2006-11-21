require("topic_list")
require("interface_builder")

local ac        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    ib:AddMsg("Clicking on the link will teleport you to tile_3 map. The used function is map:ReadyInheritedMap() which will automacally load a map depending on the source map in the same type.\nUse /maps and /mapinfo to verify it after the teleport.\nNOTE: when the map was not loaded previous, map:load will do the loading and init of the tiled map pointers - thats part of this test.")
    ib:AddLink("Teleport me to tile_3", "jmptel")			
    ac:Interface(1, ib:Build())
end

local function jmpTel()
    ac:SetPosition(me.map:ReadyInheritedMap("/dev/testmaps/map_load/testmap_mt_tile_3"), 0, 0, game.MFLAG_FIXED_POS)
    ac:Interface(-1, "")
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("jmptel", jmpTel)
tl:CheckMessage(event)

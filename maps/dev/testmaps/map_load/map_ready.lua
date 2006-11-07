require("topic_list")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

instance_name   = "/dev/testmaps/map_load/testmap_mt_tile_1"

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
	ib:AddMsg("We test here to init, load & jump to an instance by script.")

	if pl:CheckInstance(instance_name) then
		ib:AddMsg("\n\nThis instance is ACTIVE for you!\n")
		ib:AddLink("Neutralize (delete) your instance", "remInstance")			
	end
    	ib:AddLink("Load & jump to instance", "jmpinstance")
	pl:Interface(1, ib:Build())
end

local function jmpInstance()
	pl:Teleport(1, 1, pl:ReadyInstance(instance_name))
	pl:Interface(-1, "")
end

local function remInstance()
	pl:DeleteInstance(instance_name)
	topicDefault()
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)

tl:AddTopics("jmpinstance", jmpInstance)
tl:AddTopics("reminstance", remInstance)

tl:CheckMessage(event)

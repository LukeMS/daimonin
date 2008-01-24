--
-- Anonymous ranger whose only reason for existance is to 
-- tell the player where to find Taleus
--

require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

function topicDefault()
    ib:SetTitle("Greetings")
    ib:SetMsg("Hello! I'm just training here.\n\n")
    ib:AddMsg("I guess you are looking for ^Taleus^.\n\n")

    pl:Interface(1, ib:Build())
end

--
-- Rumors etc
--

local function topicTaleus()
    ib:SetTitle("Taleus")
    ib:AddMsg("\n\nTaleus is the one you want to see for archery training.")
    ib:AddMsg("\n\nHe's not here for the moment, though. He went to ^Stonehaven^ to get some supplies. If you really must see him, he is probably in the shop in Stonehaven.")
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

local function topicStonehaven()
    ib:SetTitle("Stonehaven")
    ib:AddMsg("\n\nStonehaven is the big castle nearby.")
    ib:AddMsg("\n\nTo get there exit the guildhall compound through the western gates and follow the road until the first major intersection. There you turn north and keep going until you reach the Stonehaven gates.")
    ib:AddMsg("\n\nIt can be a bit rough for an inexperienced traveler, though. There are still some wild animals in the area, and the occasional ogre raiding party.")
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:SetDefault(topicDefault)
tl:AddTopics("taleus", topicTaleus) 
tl:AddTopics("stonehaven", topicStonehaven) 
tl:CheckMessage(event)

--
-- Gaffrey will give the Mine Shaft key to a qualifying player.
--

require("topic_list")
require("quest_manager")
require("interface_builder")

local pl = event.activator
local me = event.me

local ds = DataStore("guildhall_mineshaft", pl)
local spoken = ds:Get("spoken")
local visited = ds:Get("visited")

local req_level  = 3

local key_name   = "mine shaft key"
local key_string = "mine_shaft_key"
local key = pl:CheckInventory(1, nil, "mine shaft key", nil, -1)

local key_given = false

local ib = InterfaceBuilder()
ib:SetTitle("Gaffrey")

local quest_list = {"Mouse Hunt", "The Mercenary Guild Quest", "Rusty Rod Retrieval"}
local quest_who  = {"Fanrir", "Cashin", "Jahrlen"}
local quest_max  = 3;

local function checkQuests()
    local quest_num = 0
    for i = 1, quest_max do
        if QuestManager(pl, quest_list[i]):GetStatus() < game.QSTAT_DONE then
            break
        end
        quest_num = quest_num + 1
    end
    return quest_num
end

-- Tell player he gets the key and create no-drop key in player's inventory
local function allowKey()
    ib:AddMsg("so here's the key.")
    ib:AddMsg("\n\nBut please leave it in the lock, so I can pick it up later.")
    ib:AddMsg("\n\nOh, and be very careful! There are lots of nasty ")
    ib:AddMsg("creatures lurking in there. Best to take plenty of booze with you, ")
    ib:AddMsg("so you can recover quickly.")
    pl:Sound(0, 0, 2, 0)
    local key = pl:CreateObjectInside("key_brown", game.IDENTIFIED, 1)
    key.name = key_name
    key.slaying = key_string
    key.f_no_drop = 1
    key_given = true
end

local function topicDefault()
    local q = checkQuests()
    ib:SetHeader("st_001", me)
    if spoken then
        ib:SetMsg("\n\nWelcome back, friend!\n\n")
    else
        ds:Set("spoken", true)
        ib:SetMsg("\n\nHello, friend!\n\n")
    end

    ib:AddMsg("I'm practising hard for the archery competition.\n\n")
    ib:AddMsg("This year I'm going to beat ^Taleus^!")

    if visited and (key == nil) then
        ib:AddMsg("\n\nCan I help you in any specific way?")
        ib:AddLink("Ask about the mine shaft key", "key")    
    end
end

local function topicTaleus()
    ib:SetHeader("st_004", me)
    ib:SetTitle("Taleus")
    ib:AddMsg("\n\nTaleus is the one you want to see for archery training.")
    ib:AddMsg("\n\nHe's not here for the moment, though. He went to ^Stonehaven^ to get some supplies. If you really must see him, he is probably in the shop in Stonehaven.")
    ib:SetButton("Back", "hi")
end

local function topicStonehaven()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Stonehaven")
    ib:AddMsg("\n\nStonehaven is the big castle nearby.")
    ib:AddMsg("\n\nTo get there exit the guildhall compound through the western gates and follow the road until the first major intersection. There you turn north and keep going until you reach the Stonehaven gates.")
    ib:AddMsg("\n\nIt can be a bit rough for an inexperienced traveler, though. There are still some wild animals in the area, and the occasional ogre raiding party.")
    ib:SetButton("Back", "hi")
end

local function topicKey()
    ib:SetHeader("st_002", me)
    ib:SetMsg("\n\nWell, I do have an old key that fits, but I am under strict ")
    ib:AddMsg("instructions not to let newbies anywhere near that dangerous place.\n\n")
    local q = checkQuests()
    if q < quest_max then
        ib:AddMsg("Since you've not even completed the basic quests, ")
        ib:AddMsg("I can't possibly let you have it yet!")
        ib:AddMsg("\n\nGo and talk to ~"..quest_who[q+1].."~ first.")
    elseif pl.level < req_level then
        ib:AddMsg("Level "..pl.level.."?? You've got to be kidding me. Go and kill a few ")
        ib:AddMsg("more rats and come back when you've levelled up.")
        ib:AddMsg("\n\nThen I'll think about it.")
    else
        ib:AddMsg("However, you've completed the basic quests, and seem strong enough, ")
        allowKey()
    end
end

tl = TopicList()
tl:SetDefault(topicDefault)
tl:AddTopics("taleus", topicTaleus)
tl:AddTopics("stonehaven", topicStonehaven)
tl:AddTopics("key", topicKey) 
ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))

require("topic_list")
require("interface_builder")

local activator = event.activator
local me = event.me
local msg = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
ib:SetTitle("Tavern")
ib:SetMessage("Hello! I am " .. me.name ..".\n\nWelcome to our tavern.\nWe serve all kinds of booze here.\n\nWhat you want?")
ib:AddLink("Buy water for 2 copper", "buy water")
ib:AddLink("Buy booze for 7 copper", "buy booze")
activator:Interface(1, ib:Build())
end

local function topicBuy()
local words = string.split(event.message)
ib:SetTitle("Here is my Offer")
if words[2]=="water" then
ib:SetMessage("One °water° will cost you 2 copper")
ib:SetAccept(nil, "water") 
ib:AddIcon("water", "water.101", "")
else
ib:SetMessage("One °booze° will cost you 7 copper")
ib:SetAccept(nil, "booze") 
ib:AddIcon("booze", "booze.101", "")
end
ib:AddToMessage(".\n\nYou have " .. activator:ShowCost(activator:GetMoney()) .. ".\n\nYou want buy it?") 
ib:SetDecline(nil, "hi")
activator:Interface(1, ib:Build())
end

local function topicWater()
ib:SetTitle("Buying some Water")
if activator:PayAmount(2) == 1 then
ib:SetMessage("°** " .. me.name .. " takes your money **°\n\nOk, here is your water!")
tmp = activator:CreateObjectInside("drink_generic", 1, 1) 
activator:Write('You got one water.', game.COLOR_WHITE); 
ib:AddIcon(tmp:GetName(), tmp:GetFace(), "")
else
ib:SetMessage("You don't have enough money!")
end
ib:SetButton("Back", "Hi") 
activator:Interface(1, ib:Build())
end

local function topicBooze()
ib:SetTitle("Buying some Booze")
if activator:PayAmount(7) == 1 then
ib:SetMessage("°** " .. me.name .. " takes your money **°\n\nOk, here is your booze!")
tmp = activator:CreateObjectInside("booze_generic", 1, 1) 
ib:AddIcon(tmp:GetName(), tmp:GetFace(), "")
activator:Write('You got one booze.', game.COLOR_WHITE); 
else
ib:SetMessage("You don't have enough money!")
end
ib:SetButton("Back", "Hi") 
activator:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("buy .*", topicBuy) 
tl:AddTopics("water", topicWater) 
tl:AddTopics("booze", topicBooze) 
tl:CheckMessage(event)

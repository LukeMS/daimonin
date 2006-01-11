require("topic_list")
require("interface_builder")

local activator = event.activator
local me = event.me
local msg = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name .. " the smith")

function repairCost(item)
	local cost = 5 + item:GetRepairCost()
	return cost
end

function topicDefault()
ib:SetTitle("The Smithy")
ib:SetMessage("Hello! I am " .. me.name .. ", the smith.\n")
ib:AddToMessage("i can repair or identify your equipment.\n\n")
ib:AddToMessage("What would you like to do?")
ib:AddLink("Repair my equipment", "repair")
ib:AddLink("Identify something", "identify")
activator:Interface(1, ib:Build())
end

function topicRepair()
local flag = false
ib:SetTitle("Repair my Equipment")
ib:SetMessage("Let me check your equipment...\nPerhaps an item need a fix.\nI will tell you how much each will cost.")
tmp = activator:FindMarkedObject()
if tmp ~= nil and tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
ib:AddLink("°*M*° ".. tmp:GetName() .. '  ('.. tmp.item_condition .. '/' ..tmp.item_quality .. ')  costs: ' .. activator:ShowCost(repairCost(tmp), 1), "itemfix ".. -1)
flag = true
end
for i=0,game.EQUIP_MAX,1 do
tmp = activator:GetEquipment(i)
if tmp ~= nil and tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
ib:AddLink(tmp:GetName() .. '  ('.. tmp.item_condition .. '/' ..tmp.item_quality .. ')  costs: ' .. activator:ShowCost(repairCost(tmp), 1), "itemfix ".. i)
flag = true
end
end
if flag == false then
ib:AddToMessage("\n\n°Your equipment don't need any repair°")
end
ib:SetButton("Back", "hi") 
activator:Interface(1, ib:Build())
end

function topicItemFix()
local words = string.split(event.message)
local num = tonumber(words[2])
if words[2]=="-1" then 
tmp = activator:FindMarkedObject()
elseif num ~= nil then
tmp = activator:GetEquipment(num)
end
if tmp == nil then
topicRepair()
else
ib:SetTitle("Repairing")
if tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
ib:SetMessage("Will cost you " .. activator:ShowCost(repairCost(tmp),0))
ib:AddToMessage(".\n\nYou have " .. activator:ShowCost(activator:GetMoney()) .. ".\n\n") 
ib:AddToMessage("Should i repair it now?")
ib:AddIcon(tmp:GetName(), tmp:GetFace(), 'Condition: °'.. tmp.item_condition .. '°    Quality: °' ..tmp.item_quality .. '°')
ib:SetAccept("Repair", "fix " .. words[2])
ib:SetDecline(nil, "repair") 
else
ib:SetMessage("The item don't need any repair.")
ib:SetButton("Back", "repair") 
end
activator:Interface(1, ib:Build())
end
end

function topicFix()
local words = string.split(event.message)
local num = tonumber(words[2])
if words[2]=="-1" then 
tmp = activator:FindMarkedObject()
else
if num == nil then
topicRepair()
return
end
tmp = activator:GetEquipment(num)
end
ib:SetTitle("Pay and Repair")
if tmp == nil then
ib:SetMessage("Hm, where is the item??")
else
if tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
local qua = tmp.item_quality
if activator:PayAmount(repairCost(tmp)) == 1 then
tmp:Repair()
ib:SetMessage("°** ".. me.name .." takes your money and the item **°\n")
ib:AddToMessage("°** after some times he returns **°\n\n")
if tmp.item_quality < qua then
ib:AddToMessage("Here is your repaired stuff!\nSadly it has lost a bit in quality!\nAnything else i can do for you?\n")
else
ib:AddToMessage("Here is your repaired stuff!\nThanks you. Anything else i can do for you?\n")
end
else
ib:SetMessage("You don't have enough money!")
end
else
ib:SetMessage("The item don't need any repair.")
end
ib:AddIcon(tmp:GetName(), tmp:GetFace(), 'Condition: °'.. tmp.item_condition .. '°    Quality: °' .. tmp.item_quality .. '°')
end
ib:SetButton("Back", "repair") 
activator:Interface(1, ib:Build())
end

function topicIdentify()
ib:SetTitle("Item Identification")
ib:SetMessage("Lets see what i can do for you.\nI can °identify° a single item or all.\nI can °detect magic° or °detect curse°.\nRember you must mark the single item first.\n\n")
ib:AddToMessage(".You have " .. activator:ShowCost(activator:GetMoney()) .. ".\n\n") 
ib:AddToMessage("What would you like to do?\n")
tmp = activator:FindMarkedObject()
if tmp ~= nil then
if tmp.f_identified ~= true then
ib:AddLink("°*M*° Identify ".. tmp:GetName() .. " for 150 copper", "detect single")
else
ib:AddToMessage("\n°*M*° Your marked item is allready identified.")
end
end
ib:AddLink("Identify all for 5 silver", "detect all")
ib:AddLink("Detect magic for 50 copper", "detect magic")
ib:AddLink("Detect curse for 50 copper", "detect curse")
ib:SetButton("Back", "hi") 
activator:Interface(1, ib:Build())
end

function topicDetect()
local words = string.split(event.message)
ib:SetTitle("It will cost you")
if words[2]=="magic" then
ib:SetMessage("I can cast °Detect Magic° for 50 copper")
elseif words[2] == "all" then
ib:SetMessage("I can °Identify all° for 5 silver")
elseif words[2] == "curse" then
ib:SetMessage("I can cast °Detect Curse° for 50 copper")
else
ib:SetMessage("I can °Identify One Item° for 150 copper")
end
ib:AddToMessage("coins.\n\nYou have " .. activator:ShowCost(activator:GetMoney()) .. ".\n\nYou want me to do it now?") 
ib:SetAccept(nil, "cast " .. words[2]) 
ib:SetDecline(nil, "identify") 
activator:Interface(1, ib:Build())
end

function topicCast()
local words = string.split(event.message)
if words[2] == "magic" then
sum = 50
spell = "detect magic"
elseif words[2] == "curse" then
sum = 50
spell = "detect curse"
elseif words[2] == "all" then
sum = 500
else
sum = 150
mark = activator:FindMarkedObject(); 
if mark == nil then
topicIdentify()
return
end
end
ib:SetTitle("Identification...")
if activator:PayAmount(sum) == 1 then
if sum == 500 then
me:IdentifyItem(activator, nil, game.IDENTIFY_ALL); 
elseif sum == 150 then
me:IdentifyItem(activator, mark, game.IDENTIFY_MARKED)
else
me:CastSpell(activator, game:GetSpellNr(spell), 1, 0, "")
end
ib:SetMessage("°**" .. me.name .. " takes your money **°\n\ndone!")
else
ib:SetMessage("You don't have enough money!")
end
ib:SetButton("Back", "identify") 
activator:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("detect .*", topicDetect) 
tl:AddTopics("cast .*", topicCast) 
tl:AddTopics("repair", topicRepair) 
tl:AddTopics("itemfix .*", topicItemFix) 
tl:AddTopics("fix .*", topicFix)
tl:AddTopics("identify", topicIdentify) 
tl:CheckMessage(event)

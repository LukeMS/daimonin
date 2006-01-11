require("topic_list")
require("interface_builder")

local activator = event.activator
local me = event.me
local msg = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
ib:SetTitle("The Church of the Tabernacle")
ib:SetMessage("Hello! I am " .. me.name ..".\n\nWelcome to the church of the Tabernacle!\n\nIf you are confused by my services, i can ^explain^ it.\nYou need some of our service?")
ib:AddLink("Remove Death Sickness", "cast sick")
ib:AddLink("Remove Depletion", "cast deplete")
ib:AddLink("Cure Disease", "cast disease")
ib:AddLink("Cure Poison", "cast poison")
ib:AddLink("Remove Curse from items", "cast curse")
ib:AddLink("Remove Damnation from items", "cast damn")
ib:AddLink("Please share some food", "food")
activator:Interface(1, ib:Build())
end

local function topicExplain()
ib:SetTitle("About our Services")
ib:SetMessage("I will cast the named spells on you when you pay me the money.\nDeathsick is a stronger form of depletion.\nEverytime you die stats are depleted by death sickness.")
ib:SetButton("Back", "hi")
activator:Interface(1, ib:Build())
end

local function topicFood()
if activator.food < 150 then
activator.food = 500
me:SayTo(activator, "\nYour stomach is filled again.")
else
me:SayTo(activator, "\nYou don't look very hungry.")
end
topicDefault()
end

local function topicCast()
local words = string.split(event.message)
if words[2]=="sick" then
ib:SetTitle("Casting Remove Death Sickness")
ib:SetMessage("I can cast °Remove Deathsick° for " .. activator:ShowCost(100 + (4 * activator. level * activator.level)))
elseif words[2] == "deplete" then
ib:SetMessage("I can cast °Remove Depletion° for ".. activator:ShowCost(5 * activator.level))
elseif words[2] == "poison" then
ib:SetMessage("I will cast °Cure Poison° for " .. activator:ShowCost(5 * activator.level))
elseif words[2] == "disease" then
ib:SetMessage("I can cast °Cure Disease° for ".. activator:ShowCost(100 * activator.level))
elseif words[2] == "curse" then
ib:SetMessage("I can cast °Remove Curse° for ".. activator:ShowCost(100 * activator.level))
else
ib:SetMessage("I can cast °Remove Damnation° for " .. activator:ShowCost(100 + (3 * activator. level * activator.level)))
end
ib:AddToMessage(".\n\nYou have " .. activator:ShowCost(activator:GetMoney()) .. ".\n\nYou want me to do it now?") 
ib:SetAccept(nil, "docast " .. words[2]) 
ib:SetDecline(nil, "hi")
activator:Interface(1, ib:Build())
end

local function topicDoCast()
local words = string.split(event.message)
if words[2] == "sick" then
sum = 100 + (4 * activator. level * activator.level)
spell = "remove deathsick"
elseif words[2] == "deplete" then
sum = 5 * activator.level
spell = "remove depletion"
elseif words[2] == "poison" then
sum = 5 * activator.level
spell = "cure poison"
elseif words[2] == "disease" then
sum = 100 * activator.level
spell = "cure disease"
elseif words[2] == "curse" then
sum = 5 * activator.level
spell = "remove curse"
else
sum = 100 + (3 * activator. level * activator.level)
spell = "remove damnation"
end
ib:SetTitle("Casting " .. spell)
if activator:PayAmount(sum) == 1 then
me:CastSpell(activator, game:GetSpellNr(spell), 1, 0, "")
ib:SetMessage("°** " .. me.name .. " takes your money **°\n\ndone!")
else
ib:SetMessage("You don't have enough money!")
end
ib:SetButton("Back", "Hi") 
activator:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("explain", topicExplain) 
tl:AddTopics("food", topicFood) 
tl:AddTopics("cast .*", topicCast) 
tl:AddTopics("docast .*", topicDoCast) 
tl:CheckMessage(event)

require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me
local msg = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
ib:SetTitle("The Church of the Tabernacle")
ib:SetMsg("Hello! I am " .. me.name ..".\n\nWelcome to the church of the Tabernacle!\n\nIf you are confused by my services, i can ^explain^ it.\nYou need some of our services?")
ib:AddLink("Remove Death Sickness", "cast sick")
ib:AddLink("Remove Depletion", "cast deplete")
ib:AddLink("Restoration", "cast restore")
ib:AddLink("Cure Disease", "cast disease")
ib:AddLink("Cure Poison", "cast poison")
ib:AddLink("Remove Curse from items", "cast curse")
ib:AddLink("Remove Damnation from items", "cast damn")
ib:AddLink("Please share some food", "food")
pl:Interface(1, ib:Build())
end

local function topicExplain()
ib:SetTitle("About our Services")
ib:SetMsg("I will cast the named spells on you when you pay me the money.\nDeathsick is a stronger form of depletion.\nEverytime you die stats are depleted by death sickness.")
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
end

local function topicFood()
if pl.food < 150 then
pl.food = 500
me:SayTo(pl, "\nYour stomach is filled again.")
else
me:SayTo(pl, "\nYou don't look very hungry.")
end
topicDefault()
end

local function topicCast()
local words = string.split(event.message)
if words[2]=="sick" then
ib:SetTitle("Casting Remove Death Sickness")
ib:SetMsg("I can cast °Remove Deathsick° for " .. pl:ShowCost(100 + (4 * pl. level * pl.level)))
elseif words[2] == "deplete" then
ib:SetMsg("I can cast °Remove Depletion° for ".. pl:ShowCost(5 * pl.level))
elseif words[2] == "restore" then
ib:SetMsg("I can cast °Restoration° for 150 copper")
elseif words[2] == "poison" then
ib:SetMsg("I will cast °Cure Poison° for " .. pl:ShowCost(5 * pl.level))
elseif words[2] == "disease" then
ib:SetMsg("I can cast °Cure Disease° for ".. pl:ShowCost(100 * pl.level))
elseif words[2] == "curse" then
ib:SetMsg("I can cast °Remove Curse° for ".. pl:ShowCost(100 * pl.level))
else
ib:SetMsg("I can cast °Remove Damnation° for " .. pl:ShowCost(100 + (3 * pl. level * pl.level)))
end
ib:AddMsg(".\n\nYou have " .. pl:ShowCost(pl:GetMoney()) .. ".\n\nYou want me to do it now?") 
ib:SetAccept(nil, "docast " .. words[2]) 
ib:SetDecline(nil, "hi")
pl:Interface(1, ib:Build())
end

local function topicDoCast()
local words = string.split(event.message)
if words[2] == "sick" then
sum = 100 + (4 * pl. level * pl.level)
spell = "remove death sickness"
elseif words[2] == "deplete" then
sum = 5 * pl.level
spell = "remove depletion"
elseif words[2] == "restore" then
sum = 150
spell = "restoration"
elseif words[2] == "poison" then
sum = 5 * pl.level
spell = "cure poison"
elseif words[2] == "disease" then
sum = 100 * pl.level
spell = "cure disease"
elseif words[2] == "curse" then
sum = 5 * pl.level
spell = "remove curse"
else
sum = 100 + (3 * pl. level * pl.level)
spell = "remove damnation"
end
ib:SetTitle("Casting " .. spell)
if pl:PayAmount(sum) == 1 then
me:CastSpell(pl, game:GetSpellNr(spell), 1, 0, "")
ib:SetMsg("°** " .. me.name .. " takes your money **°\n\ndone!")
else
ib:SetMsg("You don't have enough money!")
end
ib:SetButton("Back", "Hi") 
pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("explain", topicExplain) 
tl:AddTopics("food", topicFood) 
tl:AddTopics("cast .*", topicCast) 
tl:AddTopics("docast .*", topicDoCast) 
tl:CheckMessage(event)

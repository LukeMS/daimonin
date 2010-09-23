require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me
local options = event.options

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local tavern = "our tavern"
if event.options then tavern = event.options end

local for_sale = 
{
    water = {title = "water", price = 5, icon = "water.101", arch = "drink_generic", weight = 250},
    booze = {title = "booze", price = 8, icon = "booze.101", arch = "booze_generic", weight = 300},
    food =  {title = "food",  price =10, icon = "food.101", arch = "food_generic", weight = 400}
}

local function topicDefault()
    ib:SetTitle("Tavern")
    ib:SetMsg("Hello! I am " .. me.name ..".")
    ib:AddMsg("\nWelcome to " .. tavern .. ". We serve drinks and food here.")
    ib:AddMsg("\nWhat do you want?")
    for what, data in for_sale do
        ib:AddLink("Buy  1 "..data.title.. " for "..pl:ShowCost(data.price), "buy 1 "..data.title)
        ib:AddLink("Buy 10 "..data.title.. " for "..pl:ShowCost(data.price*10), "buy 10 "..data.title)
        ib:AddLink("Buy 25 "..data.title.. " for "..pl:ShowCost(data.price*25), "buy 25 "..data.title)
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

local function topicBuy(nrof, what)
    local data = for_sale[what]
    if data == nil then
        ib:SetTitle("Sorry, we don't have that")
        ib:SetMsg("Sorry, but we don't serve that here. Can I get you " ..
                  "something else?")
        ib:SetDecline("No")
        ib:SetAccept("Yes", "hello")
    else 
        if nrof == nil then 
            nrof="1" 
        end
        if nrof <= "50" then
            ib:SetTitle("Here is my offer")
            ib:SetMsg(nrof .. " " .. data.title .. " will cost you ~" ..
                      pl:ShowCost(data.price * nrof) .. "~.")
            ib:AddMsg("\nYou have ~" .. pl:ShowCost(pl:GetMoney()) .. "~.")
            if nrof == "1" then
                ib:AddMsg("\nYou want to buy it?")
            else
                ib:AddMsg("\nYou want to buy them?")
            end
            ib:AddIcon(data.title, data.icon, "", tonumber(nrof))
            ib:SetAccept(nil, "confirm "..nrof.." "..data.title) 
            ib:SetDecline(nil, "hi")
        else
            ib:SetTitle("Tavern")
            ib:SetMsg("Sorry but we aren't a wholesaler, you can only buy " ..
                      "small amounts.")
        end
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

local function topicConfirm(nrof, what)
    ib:SetTitle("Buying some Stuff")
    local data = for_sale[what]
    if nrof == nil then
        nrof = "1"
    end
    if data == nil then
        ib:SetTitle("Sorry, we don't have that")
        ib:SetMsg("Sorry, but we don't serve that here. Can I get you " ..
                  "something else?")
        ib:SetDecline("No")
        ib:SetAccept("Yes", "hello")
    elseif nrof > "50" then
        ib:SetMsg("Sorry, but we aren't a wholesaler, you can only buy " ..
                  "small amounts.")
    elseif pl.carrying + (nrof * data.weight) >= pl:GetPlayerWeightLimit() then
        ib:SetMsg("Can you explain me, how you would carry all that stuff, " ..
                  "without collapsing?")
    elseif pl:PayAmount(data.price * nrof) == 1 then
        ib:SetMsg("|** " .. me.name .. " takes your money **|")
        ib:AddMsg("\nOk, here is your " .. data.title .. "!")
        tmp = pl:CreateObjectInsideEx(data.arch, 1, nrof) 
        ib:AddIcon(tmp.name, tmp:GetFace(), "", tonumber(nrof))
    else
        ib:SetMsg("You don't have enough money!")
    end
    ib:SetButton("Back", "Hi") 
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("buy%s+(%d*)%s*(.*)", topicBuy) 
tl:AddTopics("confirm%s+(%d*)%s*(.*)", topicConfirm) 
tl:CheckMessage(event)

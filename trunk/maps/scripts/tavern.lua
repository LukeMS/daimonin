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
    water = {title = "water", price = 2, icon = "water.101", arch = "drink_generic"},
    booze = {title = "booze", price = 7, icon = "booze.101", arch = "booze_generic"},
    food =  {title = "food",  price =10, icon = "food.101", arch = "food_generic"}
}

local function topicDefault()
    ib:SetTitle("Tavern")
    ib:SetMsg("Hello! I am " .. me.name ..".\n\nWelcome to "..tavern..".\nWe serve drinks and food here.\n\nWhat do you want?")
    for what, data in for_sale do
        ib:AddLink("Buy "..data.title.. " for "..pl:ShowCost(data.price), "buy "..data.title)
    end
    pl:Interface(1, ib:Build())
end

local function topicBuy(what)
    local data = for_sale[what]
    if data == nil then
        ib:SetTitle("Sorry, we don't have that")
        ib:SetMsg("Sorry, but we don't serve that here. Can I get you something else?")
        ib:SetDecline("No")
        ib:SetAccept("Yes", "hello")
    else
        ib:SetTitle("Here is my offer")
        ib:SetMsg("One ~"..data.title.."~ will cost you "..pl:ShowCost(data.price))    
        ib:SetAccept(nil, "confirm "..data.title) 
        ib:AddIcon(data.title, data.icon)
        ib:SetDecline(nil, "hi")
    end
    ib:AddMsg(".\n\nYou have " .. pl:ShowCost(pl:GetMoney()) .. ".\n\nYou want to buy it?") 
    pl:Interface(1, ib:Build())
end

local function topicConfirm(what)
    ib:SetTitle("Buying some Water")
    local data = for_sale[what]
    if data == nil then
        ib:SetTitle("Sorry, we don't have that")
        ib:SetMsg("Sorry, but we don't server that here. Can I get you something else")
        ib:SetDecline("No")
        ib:SetAccept("Yes", "hello")
    elseif pl:PayAmount(data.price) == 1 then
        ib:SetMsg("~** " .. me.name .. " takes your money **~\n\nOk, here is your "..data.title.."!")
        tmp = pl:CreateObjectInsideEx(data.arch, 1, 1) 
        ib:AddIcon(tmp.name, tmp:GetFace(), "")
    else
        ib:SetMsg("You don't have enough money!")
    end
    ib:SetButton("Back", "Hi") 
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("buy%s+(.*)", topicBuy) 
tl:AddTopics("confirm%s+(.*)", topicConfirm) 
tl:CheckMessage(event)

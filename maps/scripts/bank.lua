require("topic_list")
require("interface_builder")

local activator = event.activator
local me = event.me
local msg = string.lower(event.message)
local pinfo_tag = "BANK_GENERAL"

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name .. " the banker")

function getBalance(who)
local pinfo = who:GetPlayerInfo(pinfo_tag)
if pinfo == nil or pinfo.value == 0 then
return activator.name .. ", you have no money stored\n\n", 0
else
return activator.name .. ",  your balance is " .. activator:ShowCost(pinfo.value) .. ".\n\n", pinfo.value
end
end

function topicDefault()
local bmsg, balance = getBalance(activator)
local onhand = activator:GetMoney()
ib:SetTitle("At the bank")
ib:SetMessage("Hello! I am " .. me.name .. ", the banker.\n")
ib:AddToMessage("What can i do for you?\n\n")
ib:AddToMessage(bmsg)
if onhand > 0 then
ib:AddToMessage("You have on hand " .. activator:ShowCost(onhand) .. ".\n\n")
else
ib:AddToMessage("You has no money to deposit.\n\n")
end
ib:AddToMessage("Would you like to ")
if balance > 0 then ib:AddLink("make a withdrawal?", "withdrawal") end
if onhand > 0 then ib:AddLink("make a deposition?", "deposition") end
activator:Interface(1, ib:Build())
end

function topicDepositDialog()
local onhand = activator:GetMoney()
ib:SetTitle("Deposit money")
ib:SetMessage(getBalance(activator))
if onhand > 0 then
ib:AddToMessage("You have on hand " .. activator:ShowCost(onhand) .. ".\n\n")
ib:AddToMessage(
[[
Enter the amount of money you want to deposit.
For example you can enter °deposit 4 gold, 2 silver, 1 copper°
or simply °deposit all° to deposit all you money.
]])
ib:AddLink("deposit all", "deposit all")
ib:SetTextfield("deposit ")
else
ib:AddToMessage("You has no money to deposit.\n\n")
end
ib:SetButton("Back", "hi")
activator:Interface(1, ib:Build())
end

-- The actual deposit handler
function topicDeposit(what)
ib:SetTitle( "Deposit - New Balance" )
local pinfo = activator:GetPlayerInfo(pinfo_tag)
if pinfo == nil then
pinfo = activator:CreatePlayerInfo(pinfo_tag)
end
local dpose = activator:Deposit(pinfo, what)
local onhand = activator:GetMoney()
if dpose == 1 and pinfo.value ~= 0 then
ib:SetMessage( "You " .. what .. ".\n\n" )
ib:AddToMessage("Your new balance is " .. activator:ShowCost(pinfo.value) .. ".\n\n")
ib:SetButton("Ok", "hi")
else
ib:SetMessage( "You try to " .. what .. ".\n\n" )
ib:AddToMessage("But you don't have that much money!\n\n")
ib:SetButton("Back", "depositation")
end
ib:AddToMessage("You have on hand " .. activator:ShowCost(onhand) .. ".\n")
activator:Interface(1, ib:Build())
end

function topicWithdrawDialog()
ib:SetTitle("Withdraw money")
local onhand = activator:GetMoney()
local bmsg, balance = getBalance(activator)
ib:SetTitle("Withdraw money")
ib:SetMessage(bmsg)
ib:AddToMessage("You have on hand " .. activator:ShowCost(onhand) .. ".\n\n")
if balance > 0 then
ib:AddToMessage(
[[
Enter the amount of money you want to withdraw.
For example you can enter °withdraw 4 silver, 2 gold° or 
simply °withdraw all° to withdraw all you money.
]])
ib:AddLink("withdraw all", "withdraw all")
ib:SetTextfield("withdraw ")
else
ib:AddToMessage("There is no money to withdraw.\n\n")
end
ib:SetButton("Back", "hi")
activator:Interface(1, ib:Build())
end

-- The actual withdraw handler
function topicWithdraw(what)
ib:SetTitle( "Withdraw - New Balance" )
local pinfo = activator:GetPlayerInfo(pinfo_tag)
if pinfo == nil then
ib:SetMessage(activator.name .. ", you have no money stored.")
ib:SetButton("Back", "hi")
else
local wdraw = activator:Withdraw(pinfo, what)
if wdraw == 1 then
ib:SetMessage( "You " .. what .. ".\n\n" )
ib:AddToMessage("Your new balance is " .. activator:ShowCost(pinfo.value) .. ".\n\n")
ib:SetButton("Ok", "hi")
else
ib:SetMessage( "You try to " .. what .. ".\n\n" )
ib:AddToMessage("You can't withdraw that amount of money.\n\n")
ib:AddToMessage("Your balance is " .. activator:ShowCost(pinfo.value) .. ".\n\n")
ib:SetButton("Back", "withdrawal")
end
end
local onhand = activator:GetMoney()
ib:AddToMessage("You have on hand " .. activator:ShowCost(onhand) .. ".\n")
activator:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("deposition", topicDepositDialog)
tl:AddTopics("withdrawal", topicWithdrawDialog)
tl:AddTopics("(deposit .*)", topicDeposit)
tl:AddTopics("(withdraw .*)", topicWithdraw)
tl:CheckMessage(event)

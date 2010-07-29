require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me
local msg = string.lower(event.message)
local pinfo_tag = "BANK_GENERAL"

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name .. " the Banker")

function getBalance(who)
    local pinfo = who:GetPlayerInfo(pinfo_tag)
    if pinfo == nil or pinfo.value == 0 then
        return pl.name .. ", you have no money in your account.\n\n", 0
    else
        return pl.name .. ",  your balance is " .. pl:ShowCost(pinfo.value) .. ".\n\n", pinfo.value
    end
end

function topicDefault()
    ib:SetHeader("st_001", me)
    local bmsg, balance = getBalance(pl)
    local onhand = pl:GetMoney()
    ib:SetTitle("At the bank")
    ib:SetMsg("Hello! I am " .. me.name .. ", the Banker.\n")
    ib:AddMsg("What can i do for you?\n\n")
    ib:AddMsg(bmsg)
    if onhand > 0 then
        ib:AddMsg("You have on hand " .. pl:ShowCost(onhand) .. ".\n\n")
    else
        ib:AddMsg("You have no money to deposit.\n\n")
    end
    ib:AddMsg("Would you like to ")
    if balance > 0 then ib:AddLink("make a withdrawal?", "withdrawal") end
    if onhand > 0 then ib:AddLink("make a deposit?", "deposit") end
end

function topicDepositDialog()
    ib:SetHeader("st_005", me)
    local onhand = pl:GetMoney()
    ib:SetTitle("Deposit money")
    ib:SetMsg(getBalance(pl))
    if onhand > 0 then
        ib:AddMsg("You have on hand " .. pl:ShowCost(onhand) .. ".\n\n")
        ib:AddMsg(
[[
Enter the amount of money you want to deposit.
For example you can enter ~deposit 4 gold, 2 silver, 1 copper~
or simply ~deposit all~ to deposit all your money.
]])
        ib:AddLink("deposit all", "deposit all")
        ib:SetTextfield("deposit ")
    else
        ib:AddMsg("You have no money to deposit.\n\n")
    end
    ib:SetButton("Back", "hi")
end

-- The actual deposit handler
function topicDeposit(what)
    ib:SetHeader("st_005", me)
    ib:SetTitle( "Deposit - New Balance" )
    local pinfo = pl:GetPlayerInfo(pinfo_tag)
    if pinfo == nil then
        pinfo = pl:CreatePlayerInfo(pinfo_tag)
    end
    local dpose = pl:Deposit(pinfo, what)
    if dpose == 1 and pinfo.value ~= 0 then
        ib:SetMsg( "You " .. what .. ".\n\n" )
        ib:AddMsg("Your new balance is " .. pl:ShowCost(pinfo.value) .. ".\n\n")
        ib:SetButton("Ok", "hi")
    else
        ib:SetMsg( "You try to " .. what .. ".\n\n" )
        ib:AddMsg("But you don't have that much money!\n\n")
        ib:SetButton("Back", "deposit")
    end
    ib:AddMsg("You have on hand " .. pl:ShowCost(pl:GetMoney()) .. ".\n")
end

function topicWithdrawDialog()
    ib:SetHeader("st_005", me)
    ib:SetTitle("Withdraw money")
    local bmsg, balance = getBalance(pl)
    ib:SetTitle("Withdraw money")
    ib:SetMsg(bmsg)
    ib:AddMsg("You have on hand " .. pl:ShowCost(pl:GetMoney()) .. ".\n\n")
    if balance > 0 then
        ib:AddMsg(
[[
Enter the amount of money you want to withdraw.
For example you can enter ~withdraw 4 silver, 2 gold~ or 
simply ~withdraw all~ to withdraw all your money.
]])
        ib:AddLink("withdraw all", "withdraw all")
        ib:SetTextfield("withdraw ")
    else
        ib:AddMsg("There is no money to withdraw.\n\n")
    end
    ib:SetButton("Back", "hi")
end

-- The actual withdraw handler
function topicWithdraw(what)
    ib:SetHeader("st_005", me)
    ib:SetTitle( "Withdraw - New Balance" )
    local pinfo = pl:GetPlayerInfo(pinfo_tag)
    if pinfo == nil then
        ib:SetMsg(pl.name .. ", you have no money in your account.")
        ib:SetButton("Back", "hi")
    else
        local wdraw = pl:Withdraw(pinfo, what)
        if wdraw == 1 then
            ib:SetMsg( "You " .. what .. ".\n\n" )
            ib:AddMsg("Your new balance is " .. pl:ShowCost(pinfo.value) .. ".\n\n")
            ib:SetButton("Ok", "hi")
        else
            ib:SetMsg( "You try to " .. what .. ".\n\n" )
            ib:AddMsg("You can't withdraw that amount of money.\n\n")
            ib:AddMsg("Your balance is " .. pl:ShowCost(pinfo.value) .. ".\n\n")
            ib:SetButton("Back", "withdrawal")
        end
    end
    ib:AddMsg("You have on hand " .. pl:ShowCost(pl:GetMoney()) .. ".\n")
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("deposit", topicDepositDialog)
tl:AddTopics("withdrawal", topicWithdrawDialog)
tl:AddTopics("(deposit .*)", topicDeposit)
tl:AddTopics("(withdraw .*)", topicWithdraw)
ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))
-- 
-- Simplified API for the NPC GUI interface
-- 

-- TODO: make tostring work directly in a call to foo:Interface(1, ib)

InterfaceBuilder = {}

-- Set window header
function InterfaceBuilder:SetHeader(face, title)
if type(face) == 'userdata' then
face = face:GetFace()
end
self.header = { face = face, title = title }
end

-- Set message title
function InterfaceBuilder:SetTitle(title)
self.message.title = title
end

-- Replace message body
function InterfaceBuilder:SetMessage(body)
self.message.body = body
end

-- Append text to message body
function InterfaceBuilder:AddToMessage(text)
self.message.body = self.message.body .. text
end

-- Add a link line
function InterfaceBuilder:AddLink(title, command)
table.insert(self.tags, { type = 'link', title = title, command = command })
end

-- Set reward
function InterfaceBuilder:SetReward(title, body, copper, silver, gold, mithril)
self.reward = { title = title, body = body, copper = copper,  silver = silver, gold = gold, mithril = mithril }
end

-- Add a (reward) icon
function InterfaceBuilder:AddIcon(title, face, body, mode)
table.insert(self.tags, { type = 'icon', title = title, face = face, body = body, mode = mode })
end

-- Set decline button
function InterfaceBuilder:SetDecline(title, command)
self.decline = { title = title, command = command }
end

-- Set accept button
function InterfaceBuilder:SetAccept(title, command)
self.accept = { title = title, command = command }
end

-- Set textfield contents
function InterfaceBuilder:SetTextfield(contents)
self.textfield = { contents = contents }
end

-- Generate the NPC GUI string from internal state
function InterfaceBuilder:Build()
local iface = ""
local function default(v, default)
if v then 
return v 
elseif default then 
return default 
else 
return '' 
end
end

local badface = ''

if self.header then
iface = iface .. '<h f="' .. default(self.header.face, badface) 
if self.header.title then
iface = iface .. '" b="' .. default(self.header.title)
end
iface = iface .. '">'
end

if self.message then
iface = iface .. '<m t="' .. default(self.message.title) .. '" b="' .. default(self.message.body) .. "\">"
end

if self.reward then
iface = iface .. '<r t="' .. default(self.reward.title) .. '" b="' .. default(self.reward.body).. '"'
if self.reward.copper ~= 0 then
iface = iface .. 'c="'.. self.reward.copper .. '"'
end
if self.reward.silver ~= 0 then
iface = iface .. 's="'.. self.reward.silver .. '"'
end
if self.reward.gold ~= 0 then
iface = iface .. 'g="'.. self.reward.gold .. '"'
end
if self.reward.mithril ~= 0 then
iface = iface .. 'm="'.. self.reward.mithril .. '"'
end
iface = iface .. '\>'
end

if self.tags then 
for _,c in self.tags do
if c.type == 'link' then
iface = iface .. '<l t="' .. default(c.title) .. '" c="' .. default("/talk " .. c.command, "/talk " .. c.title) .. '">'
elseif c.type == 'icon' then
iface = iface .. '<i m="' .. default(c.mode, "G") .. '" f="' .. default(c.face, badface) .. '" t="' .. default(c.title) .. '" b="' .. default(c.body) .. '">'
end
end
end        

if self.accept then
iface = iface .. '<a t="' .. default(self.accept.title, "Ok") .. '" c="' .. default(self.accept.command, "/talk ok") .. '">'
end

if self.decline then
iface = iface .. '<d t="' .. default(self.accept.title, "Decline") .. '" c="' .. default(self.accept.command, "/talk decline") .. '">'
end

if self.textfield then
iface = iface .. '<t b="' .. default(self.textfield.contents) .. '">'
end

return iface
end

-- Constructor
function InterfaceBuilder:New()
local obj = {tags = {}, message = { title = "", body = "" }} 
setmetatable(obj, 
{
__index = InterfaceBuilder,
__tostring = InterfaceBuilder.Build 
}
)
return obj
end    

-- Enable "InterfaceBuilder()" as an alias for "InterfaceBuilder:New()"
setmetatable(InterfaceBuilder, { __call = InterfaceBuilder.New })

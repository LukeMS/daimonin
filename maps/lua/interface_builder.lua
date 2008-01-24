-- 
-- Simplified API for the NPC GUI interface
-- 

-- TODO: make tostring work directly in a call to foo:Interface(1, ib)

InterfaceBuilder = {}

-- Set window header
function InterfaceBuilder:SetHeader(face, title)
    if type(face) == 'GameObject' then
        face = face:GetFace()
    end
    self.header = { face = face, title = title }
end

-- Set message title
function InterfaceBuilder:SetTitle(title)
    self.message.title = title
end

-- Replace message body
function InterfaceBuilder:SetMsg(body)
    self.message.body = body
end

-- Append text to message body
function InterfaceBuilder:AddMsg(text)
    self.message.body = self.message.body .. text
end

-- Add a link line
function InterfaceBuilder:AddLink(title, command)
    table.insert(self.tags, { type = 'link', title = title, command = command })
end

-- Set description & reward
function InterfaceBuilder:SetDesc(body, copper, silver, gold, mithril, title)
    self.reward = {body = body, copper = copper,  silver = silver, gold = gold, mithril = mithril, title = title }
end

-- Add a (reward) icon
function InterfaceBuilder:AddIcon(title, face, body)
    if type(face) == 'GameObject' then
        face = face:GetFace()
    end
    table.insert(self.tags, { type = 'icon', title = title, face = face, body = body})
end

-- Add a (reward) selectable icon
function InterfaceBuilder:AddSelect(title, face, body)
    if type(face) == 'GameObject' then
        face = face:GetFace()
    end
    table.insert(self.tags, { type = 'select', title = title, face = face, body = body})
end

-- Set (reward) selectable icon mode to unselectable
function InterfaceBuilder:SelectOff()
    self.select.mode = 's'
end

-- Set (reward) selectable icon mode to selectable (default)
function InterfaceBuilder:SelectOn()
    self.select.mode = 'S'
end

-- Set single button
function InterfaceBuilder:SetButton(title, command)
    self.button = { title = title, command = command }
end

-- Set accept button
function InterfaceBuilder:SetAccept(title, command)
    self.accept = { title = title, command = command }
end

-- Set decline button
function InterfaceBuilder:SetDecline(title, command)
    self.decline = { title = title, command = command }
end

-- Set textfield contents
function InterfaceBuilder:SetTextfield(contents)
    self.textfield = { contents = contents }
end

-- Add Item/Kill list from a Quest Manager
function InterfaceBuilder:AddQuestChecklist(questmanager)
    questmanager:AddItemList(self)
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
        iface = iface .. '<hf="' .. default(self.header.face, badface) 
    if self.header.title then
        iface = iface .. '"b="' .. default(self.header.title)
    end
        iface = iface .. '">'
    end

    if self.message then
        iface = iface .. '<mt="' .. default(self.message.title) .. '"b="' .. default(self.message.body) .. "\">"
    end

    if self.reward then
        iface = iface .. '<rb="' .. default(self.reward.body).. '"'
        if self.reward.title ~= nil then
            iface = iface .. 't="' .. default(self.reward.title) .. '"'
        end
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

    if self.select.mode ~= 'S' then
        self.select.mode = 's'
    end

    if self.tags then 
        for _,c in self.tags do
            if c.type == 'link' then
                iface = iface .. '<lt="' .. default(c.title) .. '"c="' .. default(c.command, c.title) .. '">'
            elseif c.type == 'icon' then
                iface = iface .. '<im="G" f="' .. default(c.face, badface) .. '"t="' .. default(c.title) .. '"b="' .. default(c.body) .. '">'
            elseif c.type == 'select' then
                iface = iface .. '<im="' .. self.select.mode .. '"f="' .. default(c.face, badface) .. '"t="' .. default(c.title) .. '"b="' .. default(c.body) .. '">'
            end
        end
    end        

    if self.button then
        iface = iface .. '<b'
        if self.button.title ~= nil then
            iface = iface .. 't="' .. self.button.title .. '"'
        end
        if self.button.command ~= nil then
            iface = iface .. 'c="' .. self.button.command .. '"'
        end
        iface = iface .. '>'
    end

    if self.accept then
        iface = iface .. '<a'
        if self.accept.title ~= nil then
            iface = iface .. 't="' .. self.accept.title .. '"'
        end
        if self.accept.command ~= nil then
            iface = iface .. 'c="' .. self.accept.command .. '"'
        end
        iface = iface .. '>'
    end

    if self.decline then
        iface = iface .. '<d'
        if self.decline.title ~= nil then
            iface = iface .. 't="' .. self.decline.title .. '"'
        end
        if self.decline.command ~= nil then
            iface = iface .. 'c="' .. self.decline.command .. '"'
        end
        iface = iface .. '>'
    end

    if self.textfield then
        iface = iface .. '<t b="' .. default(self.textfield.contents) .. '">'
    end

    return iface
end

-- Constructor
function InterfaceBuilder:New()
    local obj = {tags = {}, select = {mode ="S" }, message = { title = "", body = "" }} 
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

-------------------------------------------------------------------------------
-- interface_builder.lua
-- 
-- Simplified API for SENTInce-compliant GUIs (fully backwards-compatible --
-- which should not be read as an excuse to write bad talk scripts --
-- use SENTInce ;)).
--
-- NOTE: During development there are two special flags that can be set
-- (to true) in a script before it requires ib:
--
-- SENTInce_AWARE means certain CLIENT backwards incompatible features will be
-- used. The resulting GUI will *not* work on SENTInce-unaware clients.
--
-- SENTInce_FULL means certain CLIENT backwards incompatible features will be
-- used. The resulting GUI will *not* work on SENTInce-unaware clients.
--
-- All SENTInce-aware clients can cope whether either flag is set or not,
-- but a client compiled with the SENTInce_FULL flag *will not* work without
-- the SENTInce_FULL flag set in script or old ib.
-------------------------------------------------------------------------------
InterfaceBuilder = {}

---------------------------------------
-- Meet... da management!
---------------------------------------
-------------------
-- ib:New() constructs a new, blank ib table (the return value).
-------------------
function InterfaceBuilder:New()
    local ib = {
        header,
        message,
        description,
        icons,
        activeicons = true,
        links,
        lhsbutton,
        rhsbutton,
        textfield
    }

    setmetatable(ib, { __metatable = InterfaceBuilder,
                       __index = InterfaceBuilder,
                       __tostring = InterfaceBuilder.Build })

    return ib
end

setmetatable(InterfaceBuilder, { __call = InterfaceBuilder.New })

-------------------
-- ib:Build() generates a string based on the ib table which is built up with
-- the methods below.
-------------------
function InterfaceBuilder:Build()
    ---------
    -- interface holds the string (we want it to be an empty string, not nil,
    -- so we can concatenate it).
    ---------
    local interface = ''

    ---------
    -- Build the header (h) tag.
    ---------
    if type(self.header) == 'table' then
        local v = self.header

        interface = interface .. '<h'

        if v.face  ~= nil then
            interface = interface .. 'f="' .. tostring(v.face) .. '"'
        end

        if SENTInce_FULL then
            if v.title ~= nil then
                interface = interface .. 't="' .. tostring(v.title) .. '"'
            end
        else
            if v.title ~= nil then
                interface = interface .. 'b="' .. tostring(v.title) .. '"'
            end
        end

        interface = interface .. '>'
    end

    ---------
    -- Build the message (m) tag.
    ---------
    if type(self.message) == 'table' then
        local v = self.message

        interface = interface .. '<m'

        if v.title ~= nil then
            interface = interface .. 't="' .. tostring(v.title) .. '"'
        end

        if v.body  ~= nil then
            interface = interface .. 'b="' .. tostring(v.body) .. '"'
        end

        interface = interface .. '>'
    end

    ---------
    -- Build the description (r) tag.
    ---------
    if type(self.description) == 'table' then
        local v = self.description

        if tonumber(v.copper) == nil then
            v.copper = 0
        end

        if tonumber(v.silver) == nil then
            v.silver = 0
        end

        if tonumber(v.gold) == nil then
            v.gold = 0
        end

        if tonumber(v.mithril) == nil then
            v.mithril = 0
        end

        interface = interface .. '<r'

        if v.title ~= nil then
            interface = interface .. 't="' .. tostring(v.title) .. '"'
        end

        if v.body ~= nil then
            interface = interface .. 'b="' .. tostring(v.body).. '"'
        end

        if SENTInce_FULL then
            if v.copper ~= 0 then
                interface = interface .. '1="' .. tostring(v.copper) .. '"'
            end

            if v.silver ~= 0 then
                interface = interface .. '2="' .. tostring(v.silver) .. '"'
            end

            if v.gold ~= 0 then
                interface = interface .. '3="' .. tostring(v.gold) .. '"'
            end

            if v.mithril ~= 0 then
                interface = interface .. '4="' .. tostring(v.mithril) .. '"'
            end
        else
            if v.copper ~= 0 then
                interface = interface .. 'c="' .. tostring(v.copper) .. '"'
            end

            if v.silver ~= 0 then
                interface = interface .. 's="' .. tostring(v.silver) .. '"'
            end

            if v.gold ~= 0 then
                interface = interface .. 'g="' .. tostring(v.gold) .. '"'
            end

            if v.mithril ~= 0 then
                interface = interface .. 'm="' .. tostring(v.mithril) .. '"'
            end
        end

        interface = interface .. '>'
    end

    ---------
    -- Build the icon (i) tag(s).
    ---------
    if type(self.icons) == 'table' then
        for i, v in self.icons do
            local mode = 'G' -- default failsafe

            if tonumber(v.quantity) == nil then
                v.quantity = 1
            end

            interface = interface .. '<i'

            if v.type == 'normal' then
                if SENTInce_FULL or SENTInce_AWARE then
                    if self.activeicons == true then
                        mode = 'G'
                    else
                        mode = 'g'
                    end
                end
            elseif v.type == 'selectable' then
                if self.activeicons == true then
                    mode = 'S'
                else
                    mode = 's'
                end
            end
            interface = interface .. 'm="' .. mode .. '"'

            if v.face ~= nil then
                interface = interface .. 'f="' .. tostring(v.face) .. '"'
            end

            if v.title ~= nil then
                interface = interface .. 't="' .. tostring(v.title) .. '"'
            end

            if SENTInce_FULL or SENTInce_AWARE then
                if v.command ~= nil then
                    interface = interface .. 'c="' .. tostring(v.command) .. '"'
                end
            end

            if v.body ~= nil then
                interface = interface .. 'b="' .. tostring(v.body) .. '"'
            end

            if SENTInce_FULL then
                if v.quantity ~= nil then
                    interface = interface .. 'q="' .. tostring(v.quantity) .. '"'
                end
            else
                if v.quantity ~= nil then
                    interface = interface .. 'r="' .. tostring(v.quantity) .. '"'
                end
            end

            interface = interface .. '>'
        end
    end

    ---------
    -- Build the link (l) tag(s).
    ---------
    if type(self.links) == 'table' then
        for i, v in self.links do
            interface = interface .. '<l'

            if v.title ~= nil then
                interface = interface .. 't="' .. tostring(v.title) .. '"'
            end

            if v.command ~= nil then
                interface = interface .. 'c="' .. tostring(v.command) .. '"'
            end

            interface = interface .. '>'
        end
    end

    ---------
    -- Build the LHS button (a) tag.
    ---------
    if type(self.lhsbutton) == 'table' then
        local v = self.lhsbutton

        ---------
        -- Temp hack to force the old 'single button' behaviour.
        ---------
        if type(self.rhsbutton) ~= "table" then
            interface = interface .. '<b'
        else
            interface = interface .. '<a'
        end

        if v.title ~= nil then
            interface = interface .. 't="' .. tostring(v.title) .. '"'
        end

        if v.command ~= nil then
            interface = interface .. 'c="' .. tostring(v.command) .. '"'
        end

        interface = interface .. '>'
    end

    ---------
    -- Build the RHS button (d) tag.
    ---------
    if type(self.rhsbutton) == 'table' then
        local v = self.rhsbutton

        interface = interface .. '<d'

        if v.title ~= nil then
            interface = interface .. 't="' .. tostring(v.title) .. '"'
        end

        if v.command ~= nil then
            interface = interface .. 'c="' .. tostring(v.command) .. '"'
        end

        interface = interface .. '>'
    end

    ---------
    -- Builds the textfield (t) tag.
    ---------
    if type(self.textfield) == 'table' then
        local v = self.textfield

        interface = interface .. '<t'

        if SENTInce_FULL then
            if v.body ~= nil then
                interface = interface .. 'c="' .. tostring(v.body) .. '"'
            end
        else
            if v.body ~= nil then
                interface = interface .. 'b="' .. tostring(v.body) .. '"'
            end
        end

        interface = interface .. '>'
    end

    ---------
    -- Return the built string.
    ---------
    return interface
end

-------------------
-- ib:Unbuild(interface) degenerates interface into self. Any parts of
-- interface which do not belong are simply ignored.
-------------------
function InterfaceBuilder:Unbuild(interface)
    local _, tag

    ---------
    -- Unbuild the header (h) tag.
    ---------
    _, _, tag = string.find(interface, '<%s*h([^>]*)>')

    if tag == nil then
        ib.header = nil
    else
        local face, title

        _, _, face = string.find(tag, 'f%s*=%s*"([^"]*)"')

        if SENTInce_FULL then
            _, _, title = string.find(tag, 't%s*=%s*"([^"]*)"')
        else
            _, _, title = string.find(tag, 'b%s*=%s*"([^"]*)"')
        end

        self.header = {
            face = face,
            title = title
        }
    end

    ---------
    -- Unbuild the message (m) tag.
    ---------
    _, _, tag = string.find(interface, '<%s*m([^>]*)>')

    if tag == nil then
        ib.message = nil
    else
        local title, body

        _, _, title = string.find(tag, 't%s*=%s*"([^"]*)"')
        _, _, body = string.find(tag, 'b%s*=%s*"([^"]*)"')

        self.message = {
            title = title,
            body = body
        }
    end

    ---------
    -- Unbuild the description (r) tag.
    ---------
    _, _, tag = string.find(interface, '<%s*r([^>]*)>')

    if tag == nil then
        ib.description = nil
    else
        local title, body, copper, silver, gold, mithril

        _, _, title = string.find(tag, 't%s*=%s*"([^"]*)"')
        _, _, body = string.find(tag, 'b%s*=%s*"([^"]*)"')

        if SENTInce_FULL then
            _, _, copper = string.find(tag, '1%s*=%s*"([^"]*)"')
            _, _, silver = string.find(tag, '2%s*=%s*"([^"]*)"')
            _, _, gold = string.find(tag, '3%s*=%s*"([^"]*)"')
            _, _, mithril = string.find(tag, '4%s*=%s*"([^"]*)"')
        else
            _, _, copper = string.find(tag, 'c%s*=%s*"([^"]*)"')
            _, _, silver = string.find(tag, 's%s*=%s*"([^"]*)"')
            _, _, gold = string.find(tag, 'g%s*=%s*"([^"]*)"')
            _, _, mithril = string.find(tag, 'm%s*=%s*"([^"]*)"')
        end

        self.description = {
            title = title,
            body = body,
            copper = copper,
            silver = silver,
            gold = gold,
            mithril = mithril
        }
    end

    ---------
    -- Unbuild the icon (i) tag(s).
    ---------
    self.icons = nil

    for tag in string.gfind(interface, '<%s*i([^>]*)>') do
        local mode, face, title, command, body, quantity

        if type(self.icons) ~= "table" then
            self.icons = { }
        end

        _, _, mode = string.find(tag, 'm%s*=%s*"([^"]*)"')

        if mode == 'G' then
            mode = 'normal'
            self.activeicons = true
        elseif mode == 'g' then
            mode = 'normal'
            self.activeicons = false
        elseif mode == 'S' then
            mode = 'selectable'
            self.activeicons = true
        elseif mode == 's' then
            mode = 'selectable'
            self.activeicons = false
        end

        _, _, face = string.find(tag, 'f%s*=%s*"([^"]*)"')
        _, _, title = string.find(tag, 't%s*=%s*"([^"]*)"')
        _, _, command = string.find(tag, 'c%s*=%s*"([^"]*)"')
        _, _, body = string.find(tag, 'b%s*=%s*"([^"]*)"')

        if SENTInce_FULL then
            _, _, quantity = string.find(tag, 'q%s*=%s*"([^"]*)"')
        else
            _, _, quantity = string.find(tag, 'r%s*=%s*"([^"]*)"')
        end

        table.insert(self.icons, { type = mode,
                                   face = face,
                                   title = title,
                                   command = command,
                                   body = body,
                                   quantity = tonumber(quantity) })
    end

    ---------
    -- Unbuild the link (l) tag(s).
    ---------
    self.links = nil

    for tag in string.gfind(interface, '<%s*l([^>]*)>') do
        local title, command

        if type(self.links) ~= "table" then
            self.links = { }
        end

        _, _, title = string.find(tag, 't%s*=%s*"([^"]*)"')
        _, _, command = string.find(tag, 'c%s*=%s*"([^"]*)"')

        table.insert(self.links, { title = title,
                                   command = command })
    end

     ---------
     -- Unbuild the LHS button (a or b) tag.
     ---------
     _, _, tag = string.find(interface, '<%s*a([^>]*)>')

     if tag == nil then
         _, _, tag = string.find(interface, '<%s*b([^>]*)>')
     end

     if tag ~= nil then
        local title, command

        _, _, title = string.find(tag, 't%s*=%s*"([^"]*)"')
        _, _, command = string.find(tag, 'c%s*=%s*"([^"]*)"')

        self.lhsbutton = {
            title = title,
            command = command
        }
    end

     ---------
     -- Unbuild the RHS button (d) tag.
     ---------
     _, _, tag = string.find(interface, '<%s*d([^>]*)>')

     if tag ~= nil then
        local title, command

        _, _, title = string.find(tag, 't%s*=%s*"([^"]*)"')
        _, _, command = string.find(tag, 'c%s*=%s*"([^"]*)"')

        self.rhsbutton = {
            title = title,
            command = command
        }
    end
        
    ---------
    -- Unbuild the textfield (t) tag.
    ---------
    _, _, tag = string.find(interface, '<%s*t([^>]*)>')

    if tag ~= nil then
        local body

        if SENTInce_FULL then
            _, _, body = string.find(tag, 'c%s*=%s*"([^"]*)"')
        else
            _, _, body = string.find(tag, 'b%s*=%s*"([^"]*)"')
        end

        self.textfield = {
            body = body
        }
    end
end

-------------------
-- ib:ShowSENTInce() shows an appropriate NPC GUI layout for SENTInce.
-------------------
function InterfaceBuilder:ShowSENTInce(ev, data)
    assert(type(ev) == "boolean" or
           type(ev) == "string" or
           type(ev) == "table" or
           type(ev) == "Event" or
           ev == nil,
           "Arg #1 must be boolean, string, table, Event, or nil!")
    assert(type(data) == "boolean" or
           type(data) == "string" or
           data == nil,
           "Arg #2 must be boolean, string, or nil!")
    if type(ev) == "table" then
        assert(type(ev.activator) == "GameObject",
               "Arg #1.activator must be GameObject!")
        assert(type (ev.message) == "string",
               "Arg #1.message must be string!")
    elseif type(ev) == "string" or type(ev) == "boolean" or ev == nil then
        data = ev
        ev = event
    end

    ---------
    -- If data is true, close the GUI.
    ---------
    if data == true then
        ev.activator:Interface(-1)
    ---------
    -- Otherwise open a new GUI according to self.
    ---------
    else
        ---------
        -- Who are you talking to?
        ---------
        if type(self.header) ~= "table" then
            self:SetHeader(ev.me)
        end

        ---------
        -- What are you talking about?
        ---------
        if type(self.message) ~= "table" or self.message.title == nil then
            --local msg = string.lower(table.concat(string.split(ev.message)))
            self:SetTitle("Topic: " .. ev.message, " ")
        end

        ---------
        -- If data is a string, print it in the interface.
        ---------
        if type(data) == "string" then
            self:SetMsg(data)
        end

        ev.activator:Interface(1, self:Build())
    end
end

---------------------------------------
-- The header block.
---------------------------------------
-------------------
-- ib:SetHeader() sets the face and title of the header block.
-------------------
function InterfaceBuilder:SetHeader(face, title)
    assert(type(face) == "GameObject" or
           type(face)  == "string",
           "Arg #1 must be GameObject or string!")
    assert(type(title) == "GameObject" or
           type(title) == "string" or
           title == nil,
           "Arg #2 must be GameObject or string or nil!")
    if title == nil then
        title = face
    end
    if type(face) == "GameObject" then
        face  = face:GetFace()
    end
    if type(title) == "GameObject" then
        title = title:GetName()
    end

    self.header = {
        face = face,
        title = title 
    }
end

---------------------------------------
-- The message block.
---------------------------------------
-------------------
-- ib:SetTitle() sets the title of the message block.
-------------------
function InterfaceBuilder:SetTitle(title)
    assert(type(title) == "string",
           "Arg #1 must be string!")

    if type(self.message) == "table" then
        self.message.title = title
    else
        self.message = {
            title = title
        }
    end
end

-------------------
-- ib:SetMsg() replaces the body of the message block.
-------------------
function InterfaceBuilder:SetMsg(body)
    assert(type(body) == "string",
           "Arg #1 must be string!")

    if type(self.message) == "table" then
        self.message.body = body
    else
        self.message = {
            body = body
        }
    end
end

-------------------
-- ib:AddMsg() appends to the body of the message block.
-------------------
function InterfaceBuilder:AddMsg(body)
    assert(type(body) == "string",
           "Arg #1 must be string!")

    if type(self.message) == "table" then
        if self.message.body ~= nil then
            self.message.body = tostring(self.message.body) .. body
        else
            self.message.body = body
        end
    else
        self.message = {
            body = body
        }
    end
end

---------------------------------------
-- The description block.
---------------------------------------
-------------------
-- ib:SetSubtitle() sets the title of the description block.
-------------------
function InterfaceBuilder:SetSubtitle(title)
    assert(type(title) == "string",
           "Arg #1 must be string!")

    if type(self.description) == "table" then
        self.description.title = title
    else
        self.description = {
            title = title
        }
    end
end

-------------------
-- ib:SetDesc() replaces the body of the description block.
-------------------
function InterfaceBuilder:SetDesc(body, copper, silver, gold, mithril, title)
    assert(type(body) == "string",
           "Arg #1 must be string!")
    assert(type(copper) == "number" or
            copper == nil,
           "Arg #2 must be number or nil!")
    assert(type(silver) == "number" or
           silver == nil,
           "Arg #3 must be number or nil!")
    assert(type(gold) == "number" or
           gold == nil,
           "Arg #4 must be number or nil!")
    assert(type(mithril) == "number" or
           mithril == nil,
           "Arg #5 must be number or nil!")
    assert(type(title) == "string" or
           title  == nil,
           "Arg #6 must be string ot nil!")

    self.description = {
        title = title,
        body = body,
        copper = copper,
        silver = silver,
        gold = gold,
        mithril = mithril
    }
end

-------------------
-- ib:AddDesc() appends to the body of the description block.
-------------------
function InterfaceBuilder:AddDesc(body)
    assert(type(body) == "string",
           "Arg #1 must be string!")

    if type(self.description) == "table" then
        if self.description.body ~= nil then
            self.description.body = tostring(self.description.body) .. body
        else
            self.description.body = body
        end
    else
        self.description = {
            body = body
        }
    end
end

-------------------
-- ib:SetCoins() sets just the coins pseudo-block.
-------------------
function InterfaceBuilder:SetCoins(copper, silver, gold, mithril)
    assert(type(copper) == "number" or
           copper == nil,
           "Arg #1 must be number or nil!")
    assert(type(silver) == "number" or
           silver == nil,
           "Arg #2 must be number or nil!")
    assert(type(gold) == "number" or
           gold == nil,
           "Arg #3 must be number or nil!")
    assert(type(mithril) == "number" or
           mithril == nil,
           "Arg #4 must be number or nil!")

    if type(self.description) == "table" then
        self.description.copper = copper
        self.description.silver = silver
        self.description.gold = gold
        self.description.mithril = mithril
    else
        self.description = {
            copper = copper,
            silver = silver,
            gold = gold,
            mithril = mithril
        }
    end
end

---------------------------------------
-- The icons block.
---------------------------------------
-------------------
-- ib:_AddIcon() (internal function) does a
-- table.insert(self.icons, { type = type, title = title, command = command,
--              face = face, body = body, quantity = quantity })
-------------------
function InterfaceBuilder:_AddIcon(itype, title, command, face, body, quantity)
    if quantity == nil then
        if type(body) == "number" then
            quantity = body
            body = face
            face = command
            command = nil
        elseif body == nil then
            body = face
            face = command
            command = nil
        end
    end
    assert(type(itype) == "string",
           "Arg #1 must be string!")
    assert(type(title) == "string",
           "Arg #2 must be string!")
    assert(type(command) == "string" or
           command == nil,
           "Arg #3 must be string or nil!")
    assert(type(face) == "string" or
           type(face) == "GameObject",
           "Arg #4 must be string or GameObject!")
    assert(type(body) == "string",
           "Arg #5 must be string!")
    assert(type(quantity) == "number" or
           quantity == nil,
           "Arg #6 must be number or nil!")
    if type(face) == "GameObject" then
        face = face:GetFace()
    end

    if type(self.icons) ~= "table" then
        self.icons = { }
    end
    table.insert(self.icons, { type = itype,
                               title = title,
                               command = command,
                               face = face,
                               body = body,
                               quantity = quantity })
end

-------------------
-- ib:AddIcon() adds a normal icon.
-------------------
function InterfaceBuilder:AddIcon(title, command, face, body, quantity)
    assert(type(title) == "string",
           "Arg #1 must be string!")
    assert(type(command) == "string" or
           type(command) == "GameObject" or
           command  == nil,
           "Arg #2 must be string, GameObject, or nil!")
    assert(type(face) == "string" or
           type(face) == "GameObject",
           "Arg #3 must be string or GameObject!")
    assert(type(body) == "string" or
           type(body) == "number" or
           body == nil,
           "Arg #4 must be string, number, or nil!")
    assert(type(quantity) == "number" or
           quantity == nil,
           "Arg #5 must be number or nil!")

    self:_AddIcon("normal", title, command, face, body, quantity)
end

-------------------
-- ib:AddSelect() adds a selectable icon.
-------------------
function InterfaceBuilder:AddSelect(title, command, face, body, quantity)
    assert(type(title) == "string",
           "Arg #1 must be string!")
    assert(type(command) == "string" or
           type(command) == "GameObject" or
           command  == nil,
           "Arg #2 must be string, GameObject, or nil!")
    assert(type(face) == "string" or
           type(face) == "GameObject",
           "Arg #3 must be string or GameObject!")
    assert(type(body) == "string" or
           type(body) == "number" or
           body == nil,
           "Arg #4 must be string, number, or nil!")
    assert(type(quantity) == "number" or
           quantity == nil,
           "Arg #5 must be number or nil!")

    self:_AddIcon("selectable", title, command, face, body, quantity)
end

-------------------
-- ib:DeactivateIcons() and ib:SelectOff() deactivate icons. The latter is
-- preserved for backwards compatibility only. Its use is deprecated.
-------------------
function InterfaceBuilder:DeactivateIcons()
    self.activeicons = false
end

function InterfaceBuilder:SelectOff()
    self:DeactivateIcons()
end

-------------------
-- ib:ActivateIcons() and ib:SelectOn() activate icons. The latter is preserved
-- for backwards compatibility only. Its use is deprecated.
-------------------
function InterfaceBuilder:ActivateIcons()
    self.activeicons = true
end

function InterfaceBuilder:SelectOn()
    self:ActivateIcons()
end

---------------------------------------
-- The links block.
---------------------------------------
-------------------
-- ib:AddLink() adds a link line.
-------------------
function InterfaceBuilder:AddLink(title, command, quantity)
    assert(type(title) == "string",
           "Arg #1 must be string!")
    assert(type(command) == "string" or
           command == nil,
           "Arg #2 must be string or nil!")
    assert(type(quantity) == "number" or
           quantity == nil,
           "Arg #3 must be number or nil!")
    if command == nil then
        command = title
    end

    if quantity ~= 0 and quantity ~= nil then
        command = command .. " [" .. quantity .. "]"
    end
    if type(self.links) ~= "table" then
        self.links = { }
    end
    table.insert(self.links, { title = title,
                               command = command })
end

---------------------------------------
-- The buttons.
---------------------------------------
-------------------
-- ib:_SetButton() (internal function) writes
-- self[element] = { title = title, command = command }.
-------------------
function InterfaceBuilder:_SetButton(title, command, element)
    assert(type(title) == "string" or
           title == nil,
           "Arg #1 must be string or nil!")
    assert(type(command) == "string" or
           command == nil,
           "Arg #2 must be string or nil!")
    assert(type(element) == "string",
           "Arg #3 must be string!")
    if command == nil then
        command = title
    end

    if title ~= nil then
        if string.sub(title, 1, 1) == "#" then
            title = string.sub(title, 2)
        end
        if command == "#" then
            command = "#" .. title
        end
    end
    self[element] = {
        title = title,
        command = command
    }
end

-------------------
-- ib:SetLHSButton() sets the LHS button.
-------------------
function InterfaceBuilder:SetLHSButton(title, command)
    assert(type(title) == "string" or
           title == nil,
           "Arg #1 must be string or nil!")
    assert(type(command) == "string" or
           command == nil,
           "Arg #2 must be string or nil!")

    self:_SetButton(title, command, "lhsbutton")
end

-------------------
-- ib:SetRHSButton() sets the RHS button.
-------------------
function InterfaceBuilder:SetRHSButton(title, command)
    assert(type(title) == "string" or
           title == nil,
           "Arg #1 must be string or nil!")
    assert(type(command) == "string" or
            command == nil,
            "Arg #2 must be string or nil!")

    self:_SetButton(title, command, "rhsbutton")
end

-------------------
-- ib:SetButton() sets the LHS button and unsets the RHS button.
-------------------
function InterfaceBuilder:SetButton(title, command)
    assert(type(title) == "string" or
           title == nil,
           "Arg #1 must be string or nil!")
    assert(type(command) == "string" or
           command == nil,
           "Arg #2 must be string or nil!")

    self:_SetButton(title, command, "lhsbutton")
    self.rhsbutton = nil
end                        
                           
-------------------
-- ib:SetAccept() sets the LHS button as an accept button and the RHS button as
-- a decline button.
-------------------
function InterfaceBuilder:SetAccept(title, command)
    assert(type(title) == "string" or
           title == nil,
           "Arg #1 must be string or nil!")
    assert(type(command) == "string" or
           command == nil,
           "Arg #2 must be string or nil!")
    if command == nil then
        if title == nil then
            title = "Accept"
            command = "accept"
        else
            command = title
            if title == "#" or string.find(string.lower(title), "accept") ~= nil then
                title = "Accept"
            end
        end
    elseif title == nil then
        title = "Accept"
    end

    self:_SetButton(title, command, "lhsbutton")
    if type(self.rhsbutton) ~= "table" then
        command = string.gsub(string.lower(command), "^accept", "decline", 1)
        if command == "decline" then
            command = ""
        end
        self:_SetButton("Decline", command, "rhsbutton")
    end
end

-------------------
-- ib:SetDecline() sets the RHS button as a decline button and the LHS button
-- as an accept button.
-------------------
function InterfaceBuilder:SetDecline(title, command)
    assert(type(title) == "string" or
           title == nil,
           "Arg #1 must be string or nil!")
    assert(type(command) == "string" or
           command == nil,
           "Arg #2 must be string or nil!")
    if command == nil then
        if title == nil then
            title = "Decline"
            command = "decline"
        else
            command = title
            if title == "#" or string.find(string.lower(title), "decline") ~= nil then
                title = "Decline"
            end
        end
    elseif title == nil then
        title = "Decline"
    end

    self:_SetButton(title, command, "rhsbutton")
    if type(self.lhsbutton) ~= "table" then
        command = string.gsub(string.lower(command), "^decline", "accept", 1)
        if command == "" then
            command = "accept"
        end
        self:_SetButton("Accept", command, "lhsbutton")
    end
end

---------------------------------------
-- The textfield.
---------------------------------------
-------------------
-- ib:SetTextfield() sets the body of the textfield.
-------------------
function InterfaceBuilder:SetTextfield(body)
    assert(type(body) == "string",
           "Arg #1 must be string!")

    self.textfield = {
        body = body
    }
end

---------------------------------------
-- Special.
---------------------------------------
-------------------
-- ib:AddQuestChecklist() writes a quest status list to the message block.
-------------------
function InterfaceBuilder:AddQuestChecklist(qb, nr)
    if not QuestBuilder then
        require("quest_builder")
    end
    assert(getmetatable(qb) == QuestBuilder or
           getmetatable(qb) == QuestManager,
           "Arg #1 must be QuestBuilder or QuestManager!")
    if getmetatable(qb) == QuestBuilder then
        assert(type(nr) == "number",
               "Arg #2 must be number!")
    end

    if getmetatable(qb) == QuestBuilder then
        return qb:AddItemList(nr, self)
    else
        return qb:AddItemList(self) -- qb is really a qm
    end
end

-------------------
-- ib:AddListItem() is unfinished.
-------------------
function InterfaceBuilder:AddListItem(player, title, command, face, body, quantity)
    assert(type(player) == "string" or
           (type(player) == "GameObject" and
            player.type == game.TYPE_PLAYER),
           "Arg #1 must be string or player object!")
    assert(type(title) == "string",
           "Arg #2 must be string!")
    assert(type(command) == "string",
           "Arg #3 must be string!")
    assert(type(face) == "string",
           "Arg #4 must be string!")
    assert(type(body) == "string",
           "Arg #5 must be string!")
    assert(type(quantity) == "number" or
           quantity == nil,
           "Arg #6 must be number ot nil!")

--    local ds = DataStore("SENTInce_options", player)
--    if ds:Get("use icons") then
        self:AddSelect(title, command, face, body, quantity)
--    else
--        self:AddLink(title, command, quantity)
--    end
end

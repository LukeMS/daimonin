------------------
--
-- bitmasks.lua
--
------------------
-- A useful tool for using bitflags supplied by the server in Lua scripts. An
-- example of this (which is used throughout this file) is GMaster modes. The
-- server uses bitflags for these but Lua doesn't like that by default. This
-- library allows Lua scripts to easily check gmaster modes without a bunch
-- of nasty if-or-then statements.
------------------

Byte = { }

local default_bitsize = 8

-- Create a new byte and set each bit to 0.
local function new_byte(bitsize)

    local byte = { }
    
    for i = 1, bitsize do
    
        -- Fill our byte with 0's.
        table.insert(byte, 0)
    
    end
    
    return byte

end

-- Returns the base-two logarithm of the numerical arg.
local function log2(num)

    return math.log(num) / math.log(2)

end

-- Create a new byte with an optional bitsize (defaults to defualt_bitsize).
function Byte:New(bitsize)

    local b = {
                byte = {},
                size  = default_bitsize
              }
    
    if bitsize then
    
        b.size = bitsize
    
    end
    
    b.byte = new_byte(b.size)

    setmetatable(b, { __metatable = Byte,
                      __index = Byte })

    return b
    
end

-- Creates the constructor for the Byte class.
setmetatable(Byte, { __call = Byte.New })

------------------
-- Purpose: Creates a decimal representation of the byte table.
-- Args:    none
-- Returns: (number) The numerical representation of the byte.
-- Example: local req_modes = mode:ToNumber()
------------------
function Byte:ToNumber()

    local num = 0
    
    for i = 0, table.getn(self.byte) do
    
        if self.byte[i] == 1 then
        
            num = num + math.pow(2, i - 1)
            
        end
        
    end
    
    return num

end

------------------
-- Purpose: Puts the binary representation of a number into the byte table.
-- Args:    A decimal number to be converted to binary.
-- Returns: nil
-- Example: mode:FromNumber(pl:GetGmasterMode())
-- Notes:   By Lua's design, the byte table expands to fit the number.
------------------
function Byte:FromNumber(num)

    assert(type(num) == 'number', 'Arg #1 must be a number!')

    -- No signed bytes!
    num = math.abs(num)
    
    local bit = 0

    while (num > 0) do
    
        bit = math.floor(log2(num))
        self.byte[bit + 1] = 1
        num = num - math.pow(2, bit)
    
    end
    
    return nil

end

------------------
-- Purpose: Flags its args onto the byte table. This is similar to the &=
--              operator in C/C++.
-- Args:    One or more powers of two (bitflags) such as game.GMASTER_MODE_*
--              constants
-- Returns: nil
-- Example: mode:Mask(game.GMASTER_MODE_VOL, game.GMASTER_MODE_GM)
------------------
function Byte:Mask(...)

    if arg.n == 0 then
    
        return self
    
    end
    
    for a = 1, arg.n do

        assert(type(arg[a]) == 'number', 'Arg #' .. a .. ' must be a number!')
        
        -- If num is a power of two then log2(num) should have no decimal.
        assert(math.floor(log2(arg[a])) == log2(arg[a]), 'Arg #' .. a .. ' must be a power of 2!')
        
        -- No signed bytes!
        arg[a] = math.abs(arg[a])
    
    end
    
    -- Loop through all passed masks.
    for a = 1, arg.n do
    
        self.byte[log2(arg[a]) + 1] = 1
        
    end

    return self
    
end

------------------
-- Purpose: Checks if a specified bitflag is marked in this byte. Similar to
--              the & operator in C/C++.
-- Args:    num - A power of two that is being searched for in the byte table.
-- Returns: (boolean) Whether or not the specified flag is marked in the byte.
-- Example: if mode:Check(game.GMASTER_MODE_VOL) then ... end
------------------
function Byte:Check(num)

    assert(type(num) == 'number', 'Arg #1 must be a number!')
    
    -- If num is a power of two then log2(num) should have no decimal.
    assert(math.floor(log2(num)) == log2(num), 'Arg #1 must be a power of 2!')
    
    -- No signed bytes!
    num = math.abs(num)

    if self.byte[log2(num) + 1] == 1 then
    
        return true
        
    end
    
    return false

end

------------------
-- Purpose: Adds each bit in the byte table and its index in that table
--              to a string delimited by spaces and newlines.
-- Args:    none
-- Returns: (string) Representation of the byte table.
-- Example: mode:Debug()
--              >> 1   0
--              >> 2   1
--              >> 3   0
--              >> 4   1
------------------
function Byte:Debug()

    local str = ''
    
    for i,v in ipairs(self.byte) do
    
        str = str .. i .. '   ' .. v .. '\n'
        
    end

    return str
end
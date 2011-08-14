------------------------------
-- bitmasks.lua
------------------------------
-- Adds Lua support for bitmasks, up to 8-bit bytes are supported.
------------------------------

function _byte()
    return {0, 0, 0, 0, 0, 0, 0, 0,}
end
powers = {1, 2, 4, 8, 16, 32, 64, 128,}

function num_to_byte(num)
-- A nifty rule about powers of two:
-- The power of two directly before the represented value is always a bit in that number.
-- For example, in 55, 32 is the first power below it. So 32 is a bit of 55.
-- So, this function will loop backwards until it finds a power below num, and raises that bit to 1.
-- Then it subtracts that power from num, and loops.

    local byte = _byte()
    
    if num == 0 then return byte end
    
    --if num == 0 then return byte end
    for i = table.getn(powers), 1, -1 do
        -- First, make sure num isn't already a power of two. This can occur if either:
        -- The passed arg is already a power of two; or
        -- powers[i] is equal to the smallest bit of num.
        if powers[i] == num then
            byte[i] = 1
            break
        end
        if powers[i] < num then
            byte[i] = 1
            num = num - powers[i]
        end
    end
    
    
    return byte
end

function byte_to_num(byte)
    local num = 0
    
    for i = 0, table.getn(_byte()) do
        if byte[i] == 1 then
            num = num + math.pow(2, i - 1)
        end
    end
    
    return num
end

function mask(num, ...)
    -- Return a bitmask created by masking num with the rest of the args.
    if arg.n == 0 then return num end

    -- First, convert num to a bitwise format.
    local byte = num_to_byte(num)
    
    -- Loop through all passed masks.
    for a = 1, arg.n do
        local ind = -1
        for i = 0, table.getn(powers) do
            if powers[i] == arg[a] then
                byte[i] = 1
                break
            end
        end
    end

    return byte_to_num(byte)
end
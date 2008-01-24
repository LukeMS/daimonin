-- Stubs useful for testing lua scripts outside of Daimonin

local original_print = print

game = {}
function game:Log(lev,...) 
    original_print(unpack(arg))
end

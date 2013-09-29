-------------------------------------------------------------------------------
-- Stubs useful for testing lua scripts outside of Daimonin.
-------------------------------------------------------------------------------
if game ~= nil then
    return
end
local orig_print = print
game = {}
function game:Log(lev, ...)
    orig_print(unpack(arg))
end

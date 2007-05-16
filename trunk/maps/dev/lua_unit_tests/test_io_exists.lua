LUA_PATH="../?.lua;./?.lua"
require "plugin_init"
assert(orig_io_open == nil)

function test1()
    assert(not io.exists("Ishouldntexist"))
    assert(io.exists("test_io_exists.lua"))
end

test1()

print "All tests successful"


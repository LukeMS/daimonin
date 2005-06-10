--
-- Disable insecure (IO) functions. If you _really_ need to use any of these,
-- please contact the Daimonin development team.
--

-- security wrappers
function wrap_loadfile()
    local orig_loadfile = loadfile
    return function(filename)
        -- Don't allow null chars in filename
        assert(not string.find(filename, "%z"), "Filename contains null char")

        -- only allow loading of datastore lua files:
        assert(string.find(filename, "\.dsl$"), "Can only load datastore files")

        -- Call the original loadfile
        return orig_loadfile(filename)
    end
end

function wrap_io_open()
    local orig_io_open = io.open
    return function(filename, mode)
        -- Don't allow null chars in filename
        assert(not string.find(filename, "%z"), "Filename contains null char")

        -- only allow loading of datastore lua files:
        assert(string.find(filename, "\.dsl$"), "Can only open datastore files")

        -- Call the original loadfile
        return orig_io_open(filename, mode)
    end
end

-- Potentially dangerous functions in the base lib:
dofile = nil
loadlib = nil
loadfile = wrap_loadfile()
-- require = nil -- require is wrapped by the plugin code
-- Potentially dangerous functions in the IO lib:
io.open = wrap_io_open()
io.lines = nil
io.input = nil
io.output = nil
-- Many functions in the OS library are dangerous:
os.execute = nil
os.remove = nil
os.rename = nil
os.exit = nil
os.setlocale = nil
-- The debug library can be used to get around the wrappers
debug = {["traceback"] = debug.traceback}

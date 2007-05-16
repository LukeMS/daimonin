--
-- Test data store persistence and functionality
--
require "../data_store.lua"

os.execute("mkdir -p data/global")

function clear_memorystore()
    _data_store._global = {}
    _data_store._players = {}
end

-- Tests creation and local storage/retrieval
function test_1()
    clear_memorystore()
    os.execute("rm -f data/global/test.dsl")

    local ds = DataStore:New("test")
    ds:Set("testvalue", 42)
    assert(ds.testvalue == 42, "ds.testvalue != 42")
    
    print "test_1 passed"
end

-- Tests persistence to file
function test_2()
    clear_memorystore()
    os.execute("rm -f data/global/test.dsl")
    
    local ds = DataStore:New("test")
    assert(ds.testvalue == nil, "ds.testvalue != nil")
    ds:Set("testvalue", 42)
    assert(ds.testvalue == 42, "ds.testvalue != 42")
    _data_store.save(true)
    assert(ds.testvalue == 42, "ds.testvalue != 42")
    
    clear_memorystore()
    local ds2 = DataStore:New("test")
    ds2:Set("testvalue2", 42)
    assert(ds2.testvalue2 == 42, "ds2.testvalue2 != 42")
    assert(ds2.testvalue == 42, "ds2.testvalue != 42. Loading failed")
    
    print "test_2 passed"
end

-- Tests forced non-persistence
function test_3()
    clear_memorystore()
    os.execute("rm -f data/global/test.dsl")
    os.execute("test -f data/global/test.dsl && echo ERR: FILE NOT DELETED")
    
    -- Starts like test2
    local ds = DataStore:New("test")
    assert(ds.testvalue == nil, "ds.testvalue != nil")
    ds:Set("testvalue", 42)
    assert(ds:GetPersistence() == true, "Default persistence not true")
    ds:SetPersistence(false)
    assert(ds:GetPersistence() == false, "Persistence status not changed")
    assert(ds.testvalue == 42, "ds.testvalue != 42")
    _data_store.save(true)
    assert(ds.testvalue == 42, "ds.testvalue != 42")
    os.execute("test -f data/global/test.dsl && echo ERR: FILE SAVED")
    
    clear_memorystore()
    local ds2 = DataStore:New("test")
    ds2:Set("testvalue2", 42)
    assert(ds2.testvalue2 == 42, "ds2.testvalue2 != 42")
    assert(ds2.testvalue ~= 42, "ds2.testvalue == 42. Persistence skipping failed")

    print "test_3 passed"
end

test_1()
test_2()
test_3()

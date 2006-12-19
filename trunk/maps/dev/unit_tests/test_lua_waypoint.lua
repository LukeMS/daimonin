-- Test case for bug #0000536: using .map index on object without a map crashes server

me = event.me
activator = event.activator
activator.title = "before"
-- activator.title = me.map.orig_path -- accessing (me.map = nil).orig_path. should abort script.
activator.title = "after"
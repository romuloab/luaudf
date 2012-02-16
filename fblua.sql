drop external function lua;
declare external function LUA
    cstring(4096)
    returns cstring(4096) free_it
    entry_point 'fb_lua' module_name 'luaudf';

drop external function lua;
commit;
declare external function LUA
    varchar(4096) by descriptor, varchar(4096) by descriptor
    returns parameter 2
    entry_point 'fb_lua' module_name '/opt/firebird/UDF/luaudf.so';

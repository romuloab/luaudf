drop external function lua;
drop external function lua1;
drop external function lua2;
drop external function lua3;
drop external function lua4;
drop external function lua5;
drop external function lua6;
drop external function lua7;
drop external function lua8;
commit;
declare external function LUA
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 2
    entry_point 'fb_lua' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA1
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 3
    entry_point 'fb_lua1' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA2
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 4
    entry_point 'fb_lua2' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA3
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 5
    entry_point 'fb_lua3' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA4
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 6
    entry_point 'fb_lua4' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA5
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 7
    entry_point 'fb_lua5' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA6
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 8
    entry_point 'fb_lua6' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA7
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 9
    entry_point 'fb_lua7' module_name '/opt/firebird/UDF/luaudf.so';

declare external function LUA8
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor, 
    varchar(4096) by descriptor
    returns parameter 10
    entry_point 'fb_lua8' module_name '/opt/firebird/UDF/luaudf.so';

Lua Firebird UDF
================

luafbudf is a tiny library to use a Lua interpreter within a Firebird database. This library doesn't add scripting support to Firebird's Engine, just make it available for ad-hoc scripts.

This library registers a single external function to Firebird that can be used to eval any Lua script.

The usage is very simple. Just execute the .sql on your Firebird Database and invoke the script as:

```sql
select lua( 'return 2 + 2 * 2' ) from rdb$database
```

```
LUA
--------------------------------------------------
6
```

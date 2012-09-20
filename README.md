Lua Firebird UDF
================

luafbudf is a tiny library to use a Lua interpreter within a Firebird database. This library doesn't add scripting support to Firebird's Engine, just make it available for ad-hoc scripts.

This library registers a set of UDF functions that accept a variable number of arguments. The functions are named `lua`, `lua1`, `lua2`, ..., `lua8`, accepting zero, one, two to eight arguments (Firebird don't support varargs).

The usage is very simple. Just execute the .sql on your Firebird Database and invoke the script as:

```sql
select lua( 'return 2 + 2 * 2' ) from rdb$database;
select lua2( 'local a, b = ...; return math.pow( a, b )', 42, 3 ) from rdb$database;
select lua2( '@math.pow', 42, 3 ) from rdb$database;
```

If the script source starts with an at symbol `@`, then Lua UDF will try to traverse the path and find a suitable function:

```sql
select lua1( '@foo.bar.baz', 42 ) from rdb$database;
```

Is the equivalent to `return foo.bar.baz( 42 )`. If the field member contain dots, then a special case is provided:

```sql
select lua1( '@foo[lorem.ipsum]baz', 42 ) from rdb$database;
```

Is the equivalent to `return foo[ "lorem.ipsum" ].baz( 42 )`. There is no special syntax to method call.

As a convenience, Lua UDF exports a global table named `pkg` that is a wrapper to the require function. This way, you can do things like:

```sql
select lua2( '@pkg[socket.http]request', URL, POSTDATA ) from webservices;
```

Instead of

```sql
select lua2( 'return require "socket.http".request( ... )', URL, POSTDATA ) from webservices;
```

Whilst it may seen unnecessary, the former sugar avoids to parse the source script for every tuple in the relation.

*Important* due to a bug in the Firebird API, these UDF functions can not accept DATE, TIME nor TIMESTAMP values.

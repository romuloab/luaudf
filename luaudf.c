#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WINDOWS
    #include <windows.h>
#else
    #include <pthread.h>
#endif
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <ibase.h>
#include <math.h>

#ifndef WINDOWS
    #include <dlfcn.h>
#endif

lua_State *L = NULL;

#ifdef WINDOWS

HANDLE LM;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            LM = CreateMutex( NULL, FALSE, NULL );
        case DLL_PROCESS_DETACH:
            CloseHandle( LM );
    }
    return TRUE;
}

void mutex_lock() {
    WaitForSingleObject( LM, INFINITE );
}

void mutex_unlock() {
    ReleaseMutex( LM );
}

#else

pthread_mutex_t VLM;
pthread_mutex_t* LM = &VLM;

void __attribute__ ((constructor)) init() {
    pthread_mutex_init( LM, NULL );
}

void __attribute__ ((destructor)) fini() {
    if ( L )
        lua_close( L );
    pthread_mutex_destroy( LM );
}

void mutex_lock() {
    pthread_mutex_lock( LM );
}

void mutex_unlock() {
    pthread_mutex_unlock( LM );
}

#endif
#ifdef WINDOWS
#define API __declspec(dllexport) void __cdecl
#else
#define API void
#endif

#define DUMBDEBUG
#ifdef DUMBDEBUG
FILE *f = (FILE *)NULL;
#endif

static void pushdsc( const PARAMDSC* param ) {
    if ( param == NULL || ( param->dsc_flags & DSC_null ) == DSC_null ) {
        lua_pushnil( L );
        return;
    }
    switch( param->dsc_dtype )
    {
        case dtype_varying:
            lua_pushlstring( L, (const char*)(param->dsc_address + 2), *(ISC_SHORT*)param->dsc_address );
            break;

        case dtype_text:
            {
                const char *str = (const char*)param->dsc_address;
                int i = param->dsc_length - 1;
                for ( ; i >= 0; i-- )
                    if ( str[ i ] != ' ' ) break;

                lua_pushlstring( L, (char*)param->dsc_address, i+1 );
                break;
            }

        case dtype_short:
            if ( param->dsc_scale == 0 )
                lua_pushnumber( L, *(ISC_SHORT*)param->dsc_address );
            else
                lua_pushnumber( L, *(ISC_SHORT*)(param->dsc_address)/pow( 10, -( param->dsc_scale ) ) );
            break;

        case dtype_long:
            if ( param->dsc_scale == 0 )
                lua_pushnumber( L, *(ISC_LONG*)param->dsc_address );
            else
                lua_pushnumber( L, *(ISC_LONG*)(param->dsc_address)/pow( 10, -( param->dsc_scale ) ) );
            break;

        case dtype_int64:
            if ( param->dsc_scale == 0 )
                lua_pushnumber( L, *(ISC_INT64*)(param->dsc_address) );
            else
                lua_pushnumber( L, *(ISC_INT64*)(param->dsc_address)/pow( 10, -( param->dsc_scale ) ) );
            break;

        case dtype_real:
            lua_pushnumber( L, *(float*)param->dsc_address );
            break;

        case dtype_double:
            lua_pushnumber( L, *(double*)param->dsc_address );
            break;

        case dtype_sql_date:
        case dtype_sql_time:
        case dtype_timestamp:
        {

            /*
            struct tm t;
            if ( param->dsc_dtype == dtype_sql_date )
                isc_decode_sql_date( (ISC_DATE*)param->dsc_address, &t );
            else if ( param->dsc_dtype == dtype_sql_time )
                isc_decode_sql_time( (ISC_TIME*)param->dsc_address, &t );
            else
                isc_decode_timestamp( (ISC_TIMESTAMP*)param->dsc_address, &t );

            if ( param->dsc_dtype != dtype_sql_time )
                lua_pushfstring( L, "%.4d-%.2d-%.2d", 1900 + t.tm_year, t.tm_mon + 1, t.tm_mday );
            if ( param->dsc_dtype == dtype_timestamp )
                lua_pushstring( L, " " );
            if ( param->dsc_dtype != dtype_sql_date )
                lua_pushfstring( L, "%.2d:%.2d:%.2d", t.tm_hour, t.tm_min, t.tm_sec );

            if ( param->dsc_dtype == dtype_timestamp )
                lua_concat( L, 3 );
            */

            lua_pushstring( L, "Unsupported data type (date,time,timestamp)" );
            break;
        }
#if 0
        case dtype_blob: ??
        {
            fbdb           *db      = stmt->trans->db;
            isc_blob_handle h       = FBNULL;
            ISC_SCHAR      *buffer  = stmt_blobbuffer( stmt );
            ISC_USHORT      alen;
            int             count   = 0;

            if ( isc_open_blob2(
                    db->status,
                    &( db->handle ),
                    &( stmt->trans->handle ),
                    &h,
                    (ISC_QUAD*)param->dsc_address,
                    0,
                    NULL ) )
                return fb_error( L, db );


            for( ;; )
            {
                int st = isc_get_segment( db->status, &h, &alen, LUAFB_BLOBBUFFERSIZE, buffer );
                if ( st == isc_segstr_eof )
                    break;

                if ( st && st != isc_segment )
                {
                    isc_close_blob( db->status, &h );
                    return fb_error( L, db );
                }

                lua_pushlstring( L, buffer, alen );

                if ( ++count == 32 )
                {
                    lua_concat( L, 32 );
                    count = 1;
                }
            }


            if ( count > 1 )
                lua_concat( L, count );

            if ( isc_close_blob( db->status, &h ) )
                return fb_error( L, db );

            if ( count == 0 )
                lua_pushnil(L);

            break;
        }
#endif
        case dtype_quad:
            lua_pushstring(L,"quad");
            break;

        case dtype_array:
            lua_pushstring(L,"array");
            break;
        default:
            lua_pushnil( L );
            break;
    }
}

static int initfb( const PARAMDSC* src, PARAMDSC* ret ) {
    if ( src == NULL || ( src->dsc_flags & DSC_null ) == DSC_null ) {
        ret->dsc_flags |= DSC_null;
        ret->dsc_length = 0;
        *(short*)ret->dsc_address = (short)0;
        return 0;
    }

    mutex_lock();

    if ( !L ) {
        L = luaL_newstate();
        luaL_openlibs( L );

#ifndef WINDOWS
        /* A hack to force-load Lua's external functions */
        void *tmp = dlopen( "liblua5.1.so", RTLD_NOW | RTLD_GLOBAL );
        (void)tmp;
#endif

        luaL_dostring( L, "pkg=setmetatable({},{__index=function(_,s) return require(s) end})" );
    }
    *( src->dsc_address + src->dsc_length ) = '\0';
#ifdef DUMBDEBUG
    if ( f == NULL )
        f = fopen( "/tmp/fblog", "a" );
    fprintf( f, "cmd %s\n", (char*)src->dsc_address );fflush(f);
#endif

    char *source = (char*)src->dsc_address;

    if ( !source )
        return 0;

    if ( *source == '@' ) {
        char *p, *m, *max;
        max = source + src->dsc_length;
        p = source + 1; /* skip '@' */
        lua_pushvalue( L, LUA_GLOBALSINDEX );
        while ( *p && p <= max ) {
            m = p;

            fprintf( f, "C %c\n", *p);fflush(f);
            if ( *p == '[' ) {
                m = p + 1;
                while ( *p && p <= max && *p != ']' )
                    p++;
            }
            else
                while ( *p && p <= max && *p != '[' && *p != '.' )
                    p++;

            lua_pushlstring( L, m, p - m );
            lua_gettable( L, -2 );

            fprintf( f, "C %s %d\n", m, p - m);fflush(f);
            if ( *p == ']' || *p == '.' ) {
                p++;
                if ( p <= max && *p == '.' )
                    p++;
            }
            //if ( *p && p <= max ) p++;
            fprintf( f, "C %s %d\n", m, p - m);fflush(f);
        }
        return 1;
    }
    else
        return luaL_loadstring( L, source ) == 0;
}

static void pushresult( PARAMDSC* ret ) {
    int typ = lua_type( L, -1 );
    if ( !( typ == LUA_TNUMBER || typ == LUA_TSTRING || typ == LUA_TBOOLEAN ) ) {
        ret->dsc_flags |= DSC_null;
    }
    else {
        size_t len;
        const char *s = lua_tolstring( L, -1, &len );
        if ( len > 0x8FFF ) len = 0x8FFF;
        *(short*)ret->dsc_address = (short)len;
        memcpy( ret->dsc_address + 2, s, len + 1 );
        ret->dsc_length = len + 3;
        lua_pop( L, 1 );
    }
}

static void pusherr( const PARAMDSC* src, PARAMDSC* ret ) {
#ifdef DUMBDEBUG
    fprintf( f, "Error onscript: '%s': %s\n", src->dsc_address, lua_tostring( L, -1 ) ); fflush(f);
#endif
    lua_pop( L, 1 );
}

#if 0
API fb_lua1(
        const PARAMDSC* src,
        const PARAMDSC* param1,
        PARAMDSC* ret ) {


    if ( initfb( src, ret ) ) {
        pushdsc( param1 );
        if ( lua_pcall( L, 1, 1, 0 ) == 0 )
            pushresult( ret );
        else
            pusherr( src, ret );
    }
    else
        pusherr( src, ret );

    mutex_unlock();
}
#endif

#define BODY(COUNT,PUSH) \
    if ( initfb( src, ret ) ) { \
        PUSH; \
        if ( lua_pcall( L, COUNT, 1, 0 ) == 0 ) \
            pushresult( ret ); \
        else\
            pusherr( src, ret ); \
    }\
    else\
        pusherr( src, ret ); \
    mutex_unlock();

API fb_lua( const PARAMDSC* src,
        PARAMDSC* ret ) {
        BODY(0, {
                } ) }

API fb_lua1( const PARAMDSC* src,
        const PARAMDSC* param1,
        PARAMDSC* ret ) {
        BODY(1, {
                pushdsc(param1);
                } ) }

API fb_lua2( const PARAMDSC* src,
        const PARAMDSC* param1,
        const PARAMDSC* param2,
        PARAMDSC* ret ) {
        BODY(2, {
                pushdsc(param1);
                pushdsc(param2);
                } ) }

API fb_lua3( const PARAMDSC* src,
        const PARAMDSC* param1,
        const PARAMDSC* param2,
        const PARAMDSC* param3,
        PARAMDSC* ret ) {
        BODY(3, {
                pushdsc(param1);
                pushdsc(param2);
                pushdsc(param3);
                } ) }

API fb_lua4( const PARAMDSC* src,
        const PARAMDSC* param1,
        const PARAMDSC* param2,
        const PARAMDSC* param3,
        const PARAMDSC* param4,
        PARAMDSC* ret ) {
        BODY(4, {
                pushdsc(param1);
                pushdsc(param2);
                pushdsc(param3);
                pushdsc(param4);
                } ) }

API fb_lua5( const PARAMDSC* src,
        const PARAMDSC* param1,
        const PARAMDSC* param2,
        const PARAMDSC* param3,
        const PARAMDSC* param4,
        const PARAMDSC* param5,
        PARAMDSC* ret ) {
        BODY(5, {
                pushdsc(param1);
                pushdsc(param2);
                pushdsc(param3);
                pushdsc(param4);
                pushdsc(param5);
                } ) }

API fb_lua6( const PARAMDSC* src,
        const PARAMDSC* param1,
        const PARAMDSC* param2,
        const PARAMDSC* param3,
        const PARAMDSC* param4,
        const PARAMDSC* param5,
        const PARAMDSC* param6,
        PARAMDSC* ret ) {
        BODY(6, {
                pushdsc(param1);
                pushdsc(param2);
                pushdsc(param3);
                pushdsc(param4);
                pushdsc(param5);
                pushdsc(param6);
                } ) }

API fb_lua7( const PARAMDSC* src,
        const PARAMDSC* param1,
        const PARAMDSC* param2,
        const PARAMDSC* param3,
        const PARAMDSC* param4,
        const PARAMDSC* param5,
        const PARAMDSC* param6,
        const PARAMDSC* param7,
        PARAMDSC* ret ) {
        BODY(7, {
                pushdsc(param1);
                pushdsc(param2);
                pushdsc(param3);
                pushdsc(param4);
                pushdsc(param5);
                pushdsc(param6);
                pushdsc(param7);
                } ) }

API fb_lua8( const PARAMDSC* src,
        const PARAMDSC* param1,
        const PARAMDSC* param2,
        const PARAMDSC* param3,
        const PARAMDSC* param4,
        const PARAMDSC* param5,
        const PARAMDSC* param6,
        const PARAMDSC* param7,
        const PARAMDSC* param8,
        PARAMDSC* ret ) {
        BODY(8, {
                pushdsc(param1);
                pushdsc(param2);
                pushdsc(param3);
                pushdsc(param4);
                pushdsc(param5);
                pushdsc(param6);
                pushdsc(param7);
                pushdsc(param8);
                } ) }

// Firebird doesn't support UDFs with more than 10 arguments ( source, result, 8 arguments )

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

#define DUMBDEBUG
#ifdef DUMBDEBUG
FILE *f = (FILE *)NULL;
#endif

#ifdef WINDOWS
__declspec(dllexport) void __cdecl
#else
void
#endif
fb_lua( const PARAMDSC* src, PARAMDSC* ret ) {

    if ( src == NULL || ( src->dsc_flags & DSC_null ) == DSC_null ) {
        ret->dsc_flags |= DSC_null;
        ret->dsc_length = 0;
        *(short*)ret->dsc_address = (short)0;
        return;
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
    }
    *( src->dsc_address + src->dsc_length ) = '\0';
#ifdef DUMBDEBUG
    if ( f == NULL )
        f = fopen( "/tmp/fblog", "a" );
    fprintf( f, "cmd %s\n", (char*)src->dsc_address );fflush(f);
#endif
    if ( luaL_loadstring( L, (const char*)src->dsc_address ) == 0 ) {
        if ( lua_pcall( L, 0, 1, 0 ) == 0 ) {
            int typ = lua_type( L, -1 );
#ifdef DUMBDEBUG
            fprintf( f, "type %s\n", lua_typename( L, typ ));fflush(f);
#endif
            if ( !( typ == LUA_TNUMBER || typ == LUA_TSTRING || typ == LUA_TBOOLEAN ) ) {
                ret->dsc_flags |= DSC_null;
            }
            else {
                size_t len;
                const char *s = lua_tolstring( L, -1, &len );
#ifdef DUMBDEBUG
                fprintf( f, "Script result '%s': %s (%d)\n", src->dsc_address, s, len ); fflush(f);
#endif
                if ( len > 0x8FFF ) len = 0x8FFF;
                *(short*)ret->dsc_address = (short)len;
                memcpy( ret->dsc_address + 2, s, len + 1 );
                ret->dsc_length = len + 3;
                lua_pop( L, 1 );
            }
        }
        else {
#ifdef DUMBDEBUG
            fprintf( f, "Failed to exec script '%s': %s\n", src->dsc_address, lua_tostring( L, -1 ) ); fflush(f);
#endif
            lua_pop( L, 1 );
        }
    }
    else {
#ifdef DUMBDEBUG
        fprintf( f, "Could not parse script '%s': %s\n", src->dsc_address, lua_tostring( L, -1 ) ); fflush(f);
#endif
        lua_pop( L, 1 );
    }

    mutex_unlock();
}

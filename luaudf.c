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
#include <ib_util.h>

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
__declspec(dllexport) char* __cdecl
#else
char*
#endif
fb_lua( char* src ) {
    char *result = (char*)NULL;

    mutex_lock();

    if ( !L ) {
        L = luaL_newstate();
        luaL_openlibs( L );
    }
    if ( luaL_loadstring( L, src ) == 0 ) {
        if ( lua_pcall( L, 0, 1, 0 ) == 0 ) {
            size_t len;
            const char *s = lua_tolstring( L, -1, &len );
            if ( len > 4096 ) len = 4096;
            result = ib_util_malloc( len + 1 );
            memcpy( result, s, len + 1 );
            lua_pop( L, 1 );
        }
        else {
            printf( "Failed to exec script %s: %s\n", src, lua_tostring( L, -1 ) );
            lua_pop( L, 1 );
        }
    }
    else {
        printf( "Could not parse script %s: %s\n", src, lua_tostring( L, -1 ) );
        lua_pop( L, 1 );
    }

    mutex_unlock();

    return result;
}

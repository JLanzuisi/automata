#ifndef hello_lua__
#define hello_lua__

#include <lauxlib.h>
#include <lua.h>
#include <stdio.h>

static int hello_lua(lua_State *L) {
    printf("Hello from C!\n");
    return 0;
}

static const struct luaL_Reg hellolua[] = {
    {"hello_lua", hello_lua}, {NULL, NULL} /* sentinel */
};

int luaopen_hellolua(lua_State *L) {
    luaL_newlib(L, hellolua);
    return 1;
}

#endif
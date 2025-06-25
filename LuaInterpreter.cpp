extern "C"
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include <iostream>
#include <string>
#include <boost/algorithm/string/trim.hpp>

int l_exit(lua_State* L)
{
    const char* exitMsg = luaL_optstring(L, 1, nullptr);
    if (exitMsg)
    {
        std::cout << exitMsg << std::endl;
    }
    exit(0);
    return 0;
}

int l_clear(lua_State* L)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    return 0;
}

void regCustomCommands(lua_State* L)
{
    lua_register(L, "exit", l_exit);
    lua_register(L, "clear", l_clear);
}

void luaPrintError(lua_State* L, const char* code = "nil")
{
    const char* errorMsg = lua_tostring(L, -1);
    std::cerr << "Lua error: " << errorMsg << std::endl;
    if (code != "nil") std::cerr << "Debug code: " << code << std::endl;
    lua_pop(L, 1);
    lua_settop(L, 0);
}

int main()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    regCustomCommands(L);

    while (true)
    {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);

        boost::trim(input);
        if (input == "") continue;
        else if (input.ends_with(".lua"))
        {
            int loadFileStatus = luaL_dofile(L, input.c_str());
            if (loadFileStatus != LUA_OK)
            {
                luaPrintError(L, "-1");
                continue;
            }
            continue;
        }

        int loadStatus = luaL_loadstring(L, input.c_str());
        if (loadStatus != LUA_OK) //check if valid lua code
        {
            lua_pop(L, 1);

            std::string combined = "return " + input; //if no work then try adding return
            loadStatus = luaL_loadstring(L, combined.c_str());
            if (loadStatus != LUA_OK) //check if adding returned helped
            {
                luaPrintError(L, "0");
                continue;
            }
        }

        int callStatus = lua_pcall(L, 0, LUA_MULTRET, 0); //get the results
        if (callStatus != LUA_OK)
        {
            luaPrintError(L, "1");
        }
        else
        {
            //find results from the pcall on the stack
            int resultCount = lua_gettop(L);
            for (int i = 1; i <= resultCount; i++)
            {
                if (lua_isstring(L, i)) std::cout << "=> " << lua_tostring(L, i) << std::endl;
                else if (lua_isnumber(L, i)) std::cout << "=> " << lua_tonumber(L, i) << std::endl;
                else if (lua_isboolean(L, i)) std::cout << "=> " << (lua_toboolean(L, i) ? "true" : "false") << std::endl;
                else std::cout << "=> [non-printable Lua type]" << std::endl;
            }
        }
        lua_settop(L, 0);
    }

    lua_close(L);

    return 0;
}

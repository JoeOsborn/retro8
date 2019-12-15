#include "common.h"

#if TEST_MODE

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "vm/machine.h"
#include "lua/lua.hpp"

#include <unordered_set>

using namespace retro8;
using namespace retro8::gfx;

extern retro8::Machine machine;
Machine& m = machine;

TEST_CASE("cursor([x,] [y,] [col])")
{
  Machine& m = machine;
  auto* cursor = m.memory().cursor();

  SECTION("cursor starts at 0,0")
  {
    REQUIRE((cursor->x() == 0 && cursor->y() == 0));
  }
}

TEST_CASE("camera([x,] [y])")
{
  auto* camera = m.memory().camera();

  SECTION("camera starts at 0,0")
  {
    REQUIRE((camera->x() == 0 && camera->y() == 0));
  }

  SECTION("camera is properly set by camera() function")
  {
    m.code().initFromSource("camera(20,-10)");
    auto* camera = m.memory().camera();
    REQUIRE(camera->x() == 20);
    REQUIRE(camera->y() == -10);
  }

  SECTION("camera is properly reset by camera() function")
  {
    m.code().initFromSource("camera(20,-10)");
    REQUIRE((camera->x() == 20 && camera->y() == -10));
    m.code().initFromSource("camera()");
    REQUIRE((camera->x() == 0 && camera->y() == 0));
  }

  SECTION("single argument sets x and resets y")
  {
    m.code().initFromSource("camera(20,-10)");
    REQUIRE((camera->x() == 20 && camera->y() == -10));
    m.code().initFromSource("camera(40)");
    REQUIRE((camera->x() == 40 && camera->y() == 0));
  }
}

TEST_CASE("tilemap")
{
  Machine m;

  SECTION("tilemap address space never overlap")
  {
    std::unordered_set<const sprite_index_t*> addresses;

    for (size_t y = 0; y < TILE_MAP_HEIGHT; ++y)
    {
      for (size_t x = 0; x < TILE_MAP_WIDTH; ++x)
      {
        const sprite_index_t* address = m.memory().spriteInTileMap(x, y);
        addresses.insert(address);
      }
    }

    REQUIRE(addresses.size() == TILE_MAP_WIDTH * TILE_MAP_HEIGHT);
  }
  
  SECTION("tilemap halves are inverted compared to theie coordinates")
  {
    REQUIRE(m.memory().spriteInTileMap(0, 32) - m.memory().base() == address::TILE_MAP_LOW);
    REQUIRE(m.memory().spriteInTileMap(0, 0) - m.memory().base() == address::TILE_MAP_HIGH);
    REQUIRE(address::TILE_MAP_HIGH > address::TILE_MAP_LOW);
  }
}

TEST_CASE("lua language modifications")
{
  lua_State* L = luaL_newstate();

  SECTION("!= operator")
  {
    SECTION("!= operator is compiled successfully")
    {
      const std::string code = "x = 5 != 4";

      REQUIRE(luaL_loadstring(L, code.c_str()) == 0);
      REQUIRE(lua_pcall(L, 0, 0, 0) == 0);
    }

  }

  SECTION("binary literals")
  {
    std::string literal;
    int expected = 0;
    
    SECTION("0b1")
    {
      literal = "0b1";
      expected = 0b1;
    }

    SECTION("0b100")
    {
      literal = "0b100";
      expected = 0b100;
    }

    SECTION("0b000")
    {
      literal = "0b000";
      expected = 0b000;
    }


    const std::string code = "x = " + literal + "; return x";
    REQUIRE(luaL_loadstring(L, code.c_str()) == 0);
    REQUIRE(lua_pcall(L, 0, 1, 0) == 0);
    REQUIRE(lua_tonumber(L, -1) == expected);
  }

  SECTION("compound assignment")
  {
    SECTION("+= operator")
    {
      std::string code = "x = 2; x += 4; return x";
      REQUIRE(luaL_loadstring(L, code.c_str()) == 0);
      REQUIRE(lua_pcall(L, 0, 1, 0) == 0);
      REQUIRE(lua_tonumber(L, -1) == 6);
    }


  }

  lua_close(L);

}

int testMain(int argc, char* argv[])
{
  int result = Catch::Session().run(argc, argv);
  return result;
}

#endif
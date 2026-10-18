#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <cassert>
#include <array>
#include <vector>
#include <algorithm>
#include <iostream>

constexpr float SCREEN_SIZE = 800;

constexpr int TILE_COUNT = 20;
constexpr float TILE_SIZE = SCREEN_SIZE / TILE_COUNT;

enum TileType : int
{
    GRASS,      // Marks unoccupied space, can be overwritten 
    DIRT,       // Marks the path, cannot be overwritten
    WAYPOINT,   // Marks where the path turns, cannot be overwritten
    TURRET,     // For defeating pesky enemies
    COUNT
};

struct Cell
{
    int row;
    int col;
};

struct Enemy
{
    int HP = 2;
    Color colour = RED;
    Vector2 enemy_position = { 0.0f, 0.0f };
    float enemy_speed = 250.0f;
    int curr = 0;
    int next = curr + 1;
    bool released = false;
    bool at_end = false;
};

struct Turret
{
    Vector2 turret_location = { 0.0f, 0.0f };
    int Damage = 1;
    float fire_Interval = 0.8f;
};

constexpr std::array<Cell, 4> DIRECTIONS{ Cell{ -1, 0 }, Cell{ 1, 0 }, Cell{ 0, -1 }, Cell{ 0, 1 } };

inline bool InBounds(Cell cell, int rows = TILE_COUNT, int cols = TILE_COUNT)
{
    return cell.col >= 0 && cell.col < cols && cell.row >= 0 && cell.row < rows;
}

void DrawTile(int row, int col, Color color)
{
    DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
}

void DrawTile(int row, int col, int type)
{
    Color colors[4] =
    {
        LIME,
        BEIGE,
        SKYBLUE,
        YELLOW
    };
    //assert(type >= 0 && type < COUNT);
    DrawTile(row, col, colors[type]);
}

Vector2 TileCenter(int row, int col)
{
    float x = col * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = row * TILE_SIZE + TILE_SIZE * 0.5f;
    return { x, y };
}

// Returns a collection of adjacent cells that match the search value.
std::vector<Cell> FloodFill(Cell start, int tiles[TILE_COUNT][TILE_COUNT], TileType searchValue)
{
    // "open" = "places we want to search", "closed" = "places we've already searched".
    std::vector<Cell> result;
    std::vector<Cell> open;
    bool closed[TILE_COUNT][TILE_COUNT]{};
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            // We don't want to search zero-tiles, so add them to closed!
            closed[row][col] = tiles[row][col] == 0;
        }
    }

    // Add the starting cell to the exploration queue & search till there's nothing left!
    open.push_back(start);
    while (!open.empty())
    {
        // Remove from queue and prevent revisiting
        Cell cell = open.back();
        open.pop_back();
        closed[cell.row][cell.col] = true;

        // Add to result if explored cell has the desired value
        if (tiles[cell.row][cell.col] == searchValue)
            result.push_back(cell);

        // Search neighbours
        for (Cell dir : DIRECTIONS)
        {
            Cell adj = { cell.row + dir.row, cell.col + dir.col };
            if (InBounds(adj) && !closed[adj.row][adj.col] && tiles[adj.row][adj.col] != 0)
                open.push_back(adj);
        }
    }

    return result;
}

struct Bullet
{
    Vector2 position;
    Vector2 direction;
    float speed;
    float time;
    bool enabled;
};

int main()
{

    Bullet bullet{};
    Enemy enemy; //Do I need to zero initialize these?
    enemy.released = true;
    Enemy enemy2;
    std::vector<Enemy> enemies;
    std::vector<Turret> turrets;
    float spawn_timer = 0.0f;
    int next_enemy = 0;

    for (int i = 0; i < 10; i++)
    {
            Enemy enemy10;
            enemies.push_back(enemy10);
    }



    

    int tiles[TILE_COUNT][TILE_COUNT]
    {
        //col:0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19    row:
            { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0 }, // 0                  ..that turret can't hit anything!
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 1
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 2
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 3
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 4
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 5
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 6
            { 0, 0, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0 }, // 7
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
            { 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 10
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 11
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 12
            { 0, 0, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 13
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 14
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 15
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 16
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 17
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 18
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  // 19
    };
    std::vector<Cell> waypoints = FloodFill({ 0, 12 }, tiles, WAYPOINT);
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            if (tiles[row][col] == TURRET)
            {
                Turret turret;
                turret.turret_location = TileCenter(row, col);
                turrets.push_back(turret);
            }
        }
    }


    float bullet_time_current = 0.0f;
    float bullet_time_total = 0.25f;
    std::vector<Bullet> bullets;

    //Confirming 6 turrets and not just dots on a pixel map have been added to the vector array
    std::cout << "Number of turrets: " << turrets.size() << std::endl; 
    TraceLog(LOG_INFO, "Number of turrets: %d", (int)turrets.size());

    //Confirming 10 enemies have spawned
    std::cout << "Number of enemies: " << enemies.size() << std::endl;
    TraceLog(LOG_INFO, "Number of enemies: %d", (int)enemies.size());

    
    enemy.enemy_position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
    enemy2.enemy_position = TileCenter(waypoints[enemy2.curr].row, waypoints[enemy2.curr].col);

    //DRY Violation
    enemies[0].enemy_position = TileCenter(waypoints[enemies[0].curr].row, waypoints[enemies[0].curr].col);
    enemies[1].enemy_position = TileCenter(waypoints[enemies[1].curr].row, waypoints[enemies[1].curr].col);
    enemies[2].enemy_position = TileCenter(waypoints[enemies[2].curr].row, waypoints[enemies[2].curr].col);
    enemies[3].enemy_position = TileCenter(waypoints[enemies[3].curr].row, waypoints[enemies[3].curr].col);
    enemies[4].enemy_position = TileCenter(waypoints[enemies[4].curr].row, waypoints[enemies[4].curr].col);
    enemies[5].enemy_position = TileCenter(waypoints[enemies[5].curr].row, waypoints[enemies[5].curr].col);
    enemies[6].enemy_position = TileCenter(waypoints[enemies[6].curr].row, waypoints[enemies[6].curr].col);
    enemies[7].enemy_position = TileCenter(waypoints[enemies[7].curr].row, waypoints[enemies[7].curr].col);
    enemies[8].enemy_position = TileCenter(waypoints[enemies[8].curr].row, waypoints[enemies[8].curr].col);
    enemies[9].enemy_position = TileCenter(waypoints[enemies[9].curr].row, waypoints[enemies[9].curr].col);

    //float enemy_speed = 250.0f;   // <-- 250 pixels per second
    float min_distance = enemy.enemy_speed / 60.0f; // <-- Cheating a little here, enemy2 with the same speed uses this
    min_distance *= 1.1f;
    //bool at_end = false;

    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Tower Defense");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        spawn_timer += dt;

        //if (spawn_timer >= 1.0f)
        //{
        //    enemies[0].released = true;
        //}

        if (spawn_timer >= 1.0f && next_enemy < enemies.size())
        {
            enemies[next_enemy].released = true;
            next_enemy++;
            spawn_timer = 0.0f;
        }


        if (!enemy.at_end && enemy.released) //Keep missing this line of code scanning
        {
            Vector2 from = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
            Vector2 to = TileCenter(waypoints[enemy.next].row, waypoints[enemy.next].col);
            Vector2 direction = Vector2Normalize(to - from);
            enemy.enemy_position += direction * enemy.enemy_speed * dt;
        
            // Tolorance depends on enemy speed
            if (CheckCollisionPointCircle(enemy.enemy_position, to, min_distance))
            {
                enemy.enemy_position = to;
        
                enemy.curr++;
                enemy.next++;
                enemy.at_end = enemy.curr == waypoints.size() - 1;
            }
        }

        for (int i = 0; i < 10; i++)
        {
            if (!enemies[i].at_end && enemies[i].released)
            {
                Vector2 from = TileCenter(waypoints[enemies[i].curr].row, waypoints[enemies[i].curr].col);
                Vector2 to = TileCenter(waypoints[enemies[i].next].row, waypoints[enemies[i].next].col);
                Vector2 direction = Vector2Normalize(to - from);
                enemies[i].enemy_position += direction * enemies[i].enemy_speed * dt;

                // Tolorance depends on enemy speed
                if (CheckCollisionPointCircle(enemies[i].enemy_position, to, min_distance))
                {
                    enemies[i].enemy_position = to;

                    enemies[i].curr++;
                    enemies[i].next++;
                    enemies[i].at_end = enemies[i].curr == waypoints.size() - 1;
                }
            }
        }

        bullet_time_current += dt;
        if (bullet_time_current >= bullet_time_total && IsKeyDown(KEY_SPACE))
        {
            bullet_time_current = 0.0f;
            
            Bullet bullet{};
            bullet.position = GetMousePosition();
            bullet.direction = Vector2Normalize(ENETDOWN - bullet.position);
            bullet.speed = 300.0f;
            bullet.enabled = true;

        }


        BeginDrawing();
        ClearBackground(BLACK);

        // Draw entire grid
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }

        //DrawCircleV(enemy.enemy_position, 20.0f, enemy.colour);
        //DrawCircleV(enemies[0].enemy_position, 20.0f, enemies[0].colour);
        for (int i = 0; i < 10; i++)
        {
            DrawCircleV(enemies[i].enemy_position, 20.0f, enemies[i].colour);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}


#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <vector>

constexpr int ScreenW = 1280;
constexpr int ScreenH = 720;
constexpr float Pi = 3.1415926535f;

struct Actor {
    Vector3 pos{};
    float yaw{};
    int hp{};
    float attackClock{};
};

struct Item {
    Vector3 pos{};
    int type{};
    bool taken{};
};

struct FixedCamera {
    Vector3 pos{};
    Vector3 target{};
    int zone{};
};

float distanceXZ(Vector3 a, Vector3 b) {
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dz * dz);
}

Vector3 forwardFromYaw(float yaw) {
    return {std::sin(yaw), 0.0f, std::cos(yaw)};
}

Vector3 normalizeXZ(Vector3 v) {
    float d = std::sqrt(v.x * v.x + v.z * v.z);
    if (d < 0.001f) return {};
    return {v.x / d, 0.0f, v.z / d};
}

float dotXZ(Vector3 a, Vector3 b) {
    return a.x * b.x + a.z * b.z;
}

Vector3 add(Vector3 a, Vector3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 localOffset(float x, float y, float z, float yaw) {
    float s = std::sin(yaw);
    float c = std::cos(yaw);
    return {x * c + z * s, y, -x * s + z * c};
}

bool blocked(Vector3 p) {
    if (p.x < -8.5f || p.x > 8.5f || p.z < -8.5f || p.z > 15.5f) return true;
    if (p.z > -1.0f && p.z < 1.0f && (p.x < -1.2f || p.x > 1.2f)) return true;
    if (p.x > 2.2f && p.x < 4.8f && p.z > 5.0f && p.z < 8.2f) return true;
    return false;
}

int zoneFor(Vector3 p) {
    return p.z < -1.0f ? 0 : (p.z < 6.0f ? 1 : 2);
}

void drawBlock(Vector3 center, Vector3 size, Color fill, Color wire) {
    DrawCubeV(center, size, fill);
    DrawCubeWiresV(center, size, wire);
}

void drawWard(bool hasKey, float t) {
    drawBlock({0, -0.62f, 2.5f}, {18.0f, 0.1f, 26.0f}, {42, 45, 43, 255}, {23, 25, 25, 255});
    drawBlock({-9.0f, 0.95f, 2.5f}, {0.3f, 3.1f, 26.0f}, {42, 47, 54, 255}, {17, 19, 22, 255});
    drawBlock({9.0f, 0.95f, 2.5f}, {0.3f, 3.1f, 26.0f}, {37, 43, 49, 255}, {17, 19, 22, 255});
    drawBlock({0, 0.95f, -10.5f}, {18.0f, 3.1f, 0.3f}, {58, 54, 49, 255}, {18, 18, 17, 255});
    drawBlock({0, 0.95f, 15.7f}, {18.0f, 3.1f, 0.3f}, {58, 45, 45, 255}, {22, 15, 15, 255});
    drawBlock({-4.4f, 0.95f, 0.0f}, {3.0f, 3.0f, 0.35f}, {38, 43, 46, 255}, {18, 19, 20, 255});
    drawBlock({4.4f, 0.95f, 0.0f}, {3.0f, 3.0f, 0.35f}, {38, 43, 46, 255}, {18, 19, 20, 255});
    drawBlock({3.5f, 0.3f, 6.6f}, {2.6f, 2.4f, 3.2f}, {45, 42, 39, 255}, {19, 17, 16, 255});
    drawBlock({-2.4f, 0.0f, -5.2f}, {1.3f, 1.2f, 1.3f}, {80, 62, 43, 255}, {28, 20, 14, 255});
    drawBlock({6.8f, 0.0f, 9.7f}, {1.0f, 1.0f, 1.0f}, {72, 58, 45, 255}, {27, 20, 15, 255});
    drawBlock({0.0f, 0.5f, 15.48f}, {2.2f, 1.6f, 0.18f}, hasKey ? Color{44, 88, 70, 255} : Color{88, 37, 35, 255}, {15, 14, 13, 255});

    for (float z = -8.0f; z < 15.0f; z += 4.0f) {
        DrawSphere({-8.55f, 1.8f, z}, 0.16f + std::sin(t + z) * 0.02f, {180, 150, 86, 255});
        DrawCube({-8.55f, 1.75f, z}, 0.1f, 0.45f, 0.1f, {55, 48, 38, 255});
    }

    DrawGrid(18, 1.0f);
}

void drawSurvivor(Vector3 pos, float yaw) {
    Vector3 chest = add(pos, {0.0f, 0.42f, 0.0f});
    drawBlock(chest, {0.48f, 0.78f, 0.44f}, {67, 128, 143, 255}, {18, 31, 34, 255});
    DrawSphere(add(pos, {0.0f, 0.95f, 0.0f}), 0.22f, {190, 167, 139, 255});
    DrawCylinderEx(add(pos, localOffset(-0.18f, -0.02f, 0.0f, yaw)), add(pos, localOffset(-0.24f, -0.52f, 0.04f, yaw)), 0.06f, 0.06f, 6, {34, 52, 58, 255});
    DrawCylinderEx(add(pos, localOffset(0.18f, -0.02f, 0.0f, yaw)), add(pos, localOffset(0.24f, -0.52f, 0.04f, yaw)), 0.06f, 0.06f, 6, {34, 52, 58, 255});
    DrawCylinderEx(add(pos, localOffset(-0.32f, 0.44f, 0.02f, yaw)), add(pos, localOffset(-0.55f, 0.18f, 0.38f, yaw)), 0.045f, 0.045f, 6, {41, 70, 77, 255});
    DrawCylinderEx(add(pos, localOffset(0.32f, 0.44f, 0.02f, yaw)), add(pos, localOffset(0.55f, 0.18f, 0.38f, yaw)), 0.045f, 0.045f, 6, {41, 70, 77, 255});
    DrawCylinderEx(add(pos, localOffset(0.0f, 0.48f, 0.22f, yaw)), add(pos, localOffset(0.0f, 0.48f, 0.92f, yaw)), 0.035f, 0.025f, 8, {16, 18, 19, 255});
    DrawSphere(add(pos, localOffset(0.0f, 0.48f, 0.96f, yaw)), 0.05f, {228, 211, 132, 255});
}

void drawStalker(Vector3 pos, float yaw, float t) {
    float lurch = std::sin(t * 4.0f + pos.x) * 0.06f;
    drawBlock(add(pos, {0.0f, 0.42f + lurch, 0.0f}), {0.62f, 0.9f, 0.55f}, {122, 49, 45, 255}, {34, 15, 15, 255});
    DrawSphere(add(pos, localOffset(0.04f, 1.06f + lurch, 0.02f, yaw)), 0.24f, {103, 44, 40, 255});
    DrawCylinderEx(add(pos, localOffset(-0.36f, 0.7f, 0.0f, yaw)), add(pos, localOffset(-0.72f, 0.2f, 0.22f, yaw)), 0.07f, 0.08f, 6, {86, 35, 33, 255});
    DrawCylinderEx(add(pos, localOffset(0.36f, 0.65f, 0.0f, yaw)), add(pos, localOffset(0.74f, 0.08f, -0.05f, yaw)), 0.07f, 0.08f, 6, {86, 35, 33, 255});
    DrawCylinderEx(add(pos, localOffset(-0.18f, -0.02f, 0.0f, yaw)), add(pos, localOffset(-0.34f, -0.58f, 0.12f, yaw)), 0.08f, 0.09f, 6, {67, 29, 28, 255});
    DrawCylinderEx(add(pos, localOffset(0.18f, -0.02f, 0.0f, yaw)), add(pos, localOffset(0.32f, -0.58f, -0.12f, yaw)), 0.08f, 0.09f, 6, {67, 29, 28, 255});
    DrawSphere(add(pos, localOffset(0.0f, 1.08f + lurch, 0.25f, yaw)), 0.045f, {208, 65, 54, 255});
}

void drawWardKey(Vector3 pos, float t) {
    Vector3 p = add(pos, {0.0f, 0.25f + std::sin(t * 3.0f) * 0.07f, 0.0f});
    DrawCylinderEx(add(p, {-0.36f, 0.0f, 0.0f}), add(p, {0.28f, 0.0f, 0.0f}), 0.045f, 0.045f, 8, {232, 190, 62, 255});
    drawBlock(add(p, {0.42f, 0.0f, 0.0f}), {0.24f, 0.1f, 0.12f}, {232, 190, 62, 255}, {92, 70, 20, 255});
    DrawSphere(add(p, {-0.48f, 0.0f, 0.0f}), 0.14f, {232, 190, 62, 255});
}

void drawAmmoBox(Vector3 pos, float t) {
    Vector3 p = add(pos, {0.0f, 0.16f + std::sin(t * 2.2f + pos.x) * 0.04f, 0.0f});
    drawBlock(p, {0.62f, 0.34f, 0.42f}, {94, 99, 76, 255}, {29, 34, 25, 255});
    DrawLine3D(add(p, {-0.22f, 0.19f, -0.22f}), add(p, {-0.22f, 0.19f, 0.22f}), {190, 177, 92, 255});
    DrawLine3D(add(p, {0.22f, 0.19f, -0.22f}), add(p, {0.22f, 0.19f, 0.22f}), {190, 177, 92, 255});
}

void resetGame(Actor& player, std::vector<Actor>& enemies, std::vector<Item>& items, int& ammo, bool& hasKey, bool& won, bool& dead) {
    player = {{-5.8f, 0.0f, -6.0f}, 0.1f, 100, 0.0f};
    enemies = {{{5.8f, 0.0f, 3.8f}, Pi, 45, 0.0f}, {{-5.7f, 0.0f, 12.3f}, 0.0f, 60, 0.0f}};
    items = {{{5.7f, 0.15f, -5.9f}, 0, false}, {{-6.5f, 0.15f, 8.6f}, 1, false}, {{6.1f, 0.15f, 13.5f}, 1, false}};
    ammo = 6;
    hasKey = false;
    won = false;
    dead = false;
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(ScreenW, ScreenH, "Shadow Ward - raylib lightweight 3D edition");
    SetTargetFPS(60);

    Actor player;
    std::vector<Actor> enemies;
    std::vector<Item> items;
    int ammo{};
    bool hasKey{}, won{}, dead{};
    resetGame(player, enemies, items, ammo, hasKey, won, dead);

    std::vector<FixedCamera> cams = {
        {{-1.0f, 5.0f, -12.0f}, {-4.0f, 0.6f, -3.6f}, 0},
        {{7.8f, 4.6f, 1.8f}, {-1.5f, 0.4f, 3.2f}, 1},
        {{-6.2f, 5.1f, 16.4f}, {1.2f, 0.3f, 10.5f}, 2},
    };

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.05f);
        if ((dead || won) && IsKeyPressed(KEY_R)) resetGame(player, enemies, items, ammo, hasKey, won, dead);

        if (!dead && !won) {
            float turn = (IsKeyDown(KEY_D) ? 1.0f : 0.0f) - (IsKeyDown(KEY_A) ? 1.0f : 0.0f);
            float move = (IsKeyDown(KEY_W) ? 1.0f : 0.0f) - (IsKeyDown(KEY_S) ? 1.0f : 0.0f);
            player.yaw += turn * dt * 2.5f;
            Vector3 dir = forwardFromYaw(player.yaw);
            Vector3 next = {player.pos.x + dir.x * move * dt * 3.3f, player.pos.y, player.pos.z + dir.z * move * dt * 3.3f};
            if (!blocked(next)) player.pos = next;

            for (auto& item : items) {
                if (!item.taken && distanceXZ(player.pos, item.pos) < 1.15f && IsKeyPressed(KEY_E)) {
                    item.taken = true;
                    if (item.type == 0) hasKey = true;
                    if (item.type == 1) ammo += 3;
                }
            }
            if (hasKey && player.pos.z > 14.5f && IsKeyPressed(KEY_E)) won = true;

            if (IsKeyPressed(KEY_SPACE) && ammo > 0) {
                ammo--;
                Vector3 aim = forwardFromYaw(player.yaw);
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    Vector3 to = {e.pos.x - player.pos.x, 0.0f, e.pos.z - player.pos.z};
                    float d = distanceXZ(e.pos, player.pos);
                    if (d < 7.2f && dotXZ(normalizeXZ(to), aim) > 0.72f) {
                        e.hp -= 30;
                        break;
                    }
                }
            }

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                Vector3 to = {player.pos.x - e.pos.x, 0.0f, player.pos.z - e.pos.z};
                float d = distanceXZ(e.pos, player.pos);
                e.yaw = std::atan2(to.x, to.z);
                if (d < 8.7f) {
                    Vector3 step = normalizeXZ(to);
                    Vector3 np = {e.pos.x + step.x * dt * 1.05f, e.pos.y, e.pos.z + step.z * dt * 1.05f};
                    if (!blocked(np)) e.pos = np;
                }
                e.attackClock -= dt;
                if (d < 0.95f && e.attackClock <= 0.0f) {
                    player.hp -= 18;
                    e.attackClock = 1.25f;
                    if (player.hp <= 0) dead = true;
                }
            }
        }

        int z = zoneFor(player.pos);
        FixedCamera rig = *std::find_if(cams.begin(), cams.end(), [z](const FixedCamera& c) { return c.zone == z; });
        Camera3D camera{};
        camera.position = rig.pos;
        camera.target = rig.target;
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        BeginDrawing();
        ClearBackground({7, 8, 10, 255});
        BeginMode3D(camera);
        drawWard(hasKey, GetTime());

        for (auto& item : items) {
            if (item.taken) continue;
            if (item.type == 0) {
                drawWardKey(item.pos, GetTime());
            } else {
                drawAmmoBox(item.pos, GetTime());
            }
        }
        for (auto& e : enemies) {
            if (e.hp > 0) drawStalker(e.pos, e.yaw, GetTime());
        }
        drawSurvivor(player.pos, player.yaw);
        EndMode3D();

        DrawRectangle(0, ScreenH - 96, ScreenW, 96, {8, 11, 13, 235});
        DrawText(TextFormat("HP %03i   AMMO %02i   KEY %s", std::max(0, player.hp), ammo, hasKey ? "YES" : "NO"),
                 24, ScreenH - 78, 26, {204, 224, 214, 255});
        DrawText("W/S move   A/D turn   Space fire   E interact   R restart", 24, ScreenH - 40, 20, {134, 153, 144, 255});
        DrawText(TextFormat("CAM %d", z + 1), ScreenW - 112, 24, 24, {140, 160, 150, 255});

        if (!items.empty() && !items[0].taken && distanceXZ(player.pos, items[0].pos) < 1.4f) {
            DrawText("Press E: take ward key", 420, 32, 28, {236, 210, 124, 255});
        }
        if (hasKey && player.pos.z > 14.2f) {
            DrawText("Press E: unlock service door", 386, 32, 28, {236, 210, 124, 255});
        }
        if (dead) {
            DrawRectangle(0, 0, ScreenW, ScreenH, {0, 0, 0, 120});
            DrawText("YOU DIED - press R", 448, 318, 38, {225, 72, 62, 255});
        }
        if (won) {
            DrawRectangle(0, 0, ScreenW, ScreenH, {0, 0, 0, 110});
            DrawText("ESCAPED THE SHADOW WARD - press R", 310, 318, 34, {130, 226, 170, 255});
        }
        EndDrawing();
    }

    // Let process shutdown reclaim raylib resources on this MSYS2 build.
    return 0;
}

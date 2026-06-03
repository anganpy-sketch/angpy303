#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

constexpr int ScreenW = 960;
constexpr int ScreenH = 540;
constexpr float Pi = 3.1415926535f;

struct Vec3 {
    float x{}, y{}, z{};
};

struct Vec2 {
    float x{}, y{};
};

struct Tri {
    Vec3 a, b, c;
};

struct Mesh {
    std::vector<Tri> tris;
};

struct DrawTri {
    POINT p[3]{};
    float depth{};
    COLORREF color{};
};

struct Actor {
    Vec3 pos{};
    float yaw{};
    int hp{};
    float attackClock{};
};

struct Item {
    Vec3 pos{};
    int type{};
    bool taken{};
};

struct CameraRig {
    Vec3 pos{};
    Vec3 target{};
    int zone{};
};

static bool g_running = true;
static bool g_keys[256]{};
static bool g_prevKeys[256]{};

Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator*(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }

float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float len(Vec3 a) { return std::sqrt(dot(a, a)); }
Vec3 norm(Vec3 a) {
    float l = len(a);
    return l > 0.0001f ? a * (1.0f / l) : Vec3{};
}
Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Mesh loadObj(const std::string& path) {
    Mesh mesh;
    std::ifstream file(path);
    std::vector<Vec3> verts;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream in(line);
        std::string tag;
        in >> tag;
        if (tag == "v") {
            Vec3 v;
            in >> v.x >> v.y >> v.z;
            verts.push_back(v);
        } else if (tag == "f") {
            int a{}, b{}, c{};
            in >> a >> b >> c;
            if (a > 0 && b > 0 && c > 0 && a <= (int)verts.size() && b <= (int)verts.size() && c <= (int)verts.size()) {
                mesh.tris.push_back({verts[a - 1], verts[b - 1], verts[c - 1]});
            }
        }
    }
    return mesh;
}

Vec3 rotateY(Vec3 v, float yaw) {
    float s = std::sin(yaw), c = std::cos(yaw);
    return {v.x * c - v.z * s, v.y, v.x * s + v.z * c};
}

void addMesh(std::vector<DrawTri>& out, const Mesh& mesh, Vec3 pos, float yaw, float scale, COLORREF color,
             Vec3 camPos, Vec3 right, Vec3 up, Vec3 forward) {
    for (const auto& t : mesh.tris) {
        Vec3 pts[3] = {rotateY(t.a * scale, yaw) + pos, rotateY(t.b * scale, yaw) + pos, rotateY(t.c * scale, yaw) + pos};
        Vec3 normal = norm(cross(pts[1] - pts[0], pts[2] - pts[0]));
        if (dot(normal, camPos - pts[0]) <= -0.05f) continue;

        POINT projected[3]{};
        float depths[3]{};
        bool clipped = false;
        for (int i = 0; i < 3; ++i) {
            Vec3 rel = pts[i] - camPos;
            float z = dot(rel, forward);
            if (z < 0.15f) clipped = true;
            float x = dot(rel, right);
            float y = dot(rel, up);
            float f = 390.0f / z;
            projected[i].x = (LONG)(ScreenW * 0.5f + x * f);
            projected[i].y = (LONG)(ScreenH * 0.55f - y * f);
            depths[i] = z;
        }
        if (clipped) continue;

        float light = std::clamp(dot(normal, norm({-0.35f, 0.75f, -0.45f})) * 0.55f + 0.5f, 0.22f, 1.0f);
        float fog = std::clamp(1.0f - (depths[0] + depths[1] + depths[2]) / 48.0f, 0.22f, 1.0f);
        int r = (int)(GetRValue(color) * light * fog);
        int g = (int)(GetGValue(color) * light * fog);
        int b = (int)(GetBValue(color) * light * fog);
        out.push_back({{projected[0], projected[1], projected[2]}, (depths[0] + depths[1] + depths[2]) / 3.0f, RGB(r, g, b)});
    }
}

Mesh makeBox(float sx, float sy, float sz) {
    Mesh m;
    float x = sx * 0.5f, y = sy * 0.5f, z = sz * 0.5f;
    Vec3 v[8] = {{-x,-y,-z},{x,-y,-z},{x,y,-z},{-x,y,-z},{-x,-y,z},{x,-y,z},{x,y,z},{-x,y,z}};
    int f[][3] = {{0,2,1},{0,3,2},{4,5,6},{4,6,7},{0,1,5},{0,5,4},{1,2,6},{1,6,5},{2,3,7},{2,7,6},{3,0,4},{3,4,7}};
    for (auto& face : f) m.tris.push_back({v[face[0]], v[face[1]], v[face[2]]});
    return m;
}

float dist2D(Vec3 a, Vec3 b) {
    float dx = a.x - b.x, dz = a.z - b.z;
    return std::sqrt(dx * dx + dz * dz);
}

bool blocked(Vec3 p) {
    if (p.x < -8.5f || p.x > 8.5f || p.z < -8.5f || p.z > 15.5f) return true;
    if (p.z > -1.0f && p.z < 1.0f && (p.x < -1.2f || p.x > 1.2f)) return true;
    if (p.x > 2.2f && p.x < 4.8f && p.z > 5.0f && p.z < 8.2f) return true;
    return false;
}

int zoneFor(Vec3 p) {
    return p.z < -1.0f ? 0 : (p.z < 6.0f ? 1 : 2);
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg) {
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (w < 256) g_keys[w] = true;
        return 0;
    case WM_KEYUP:
        if (w < 256) g_keys[w] = false;
        return 0;
    default:
        return DefWindowProc(hwnd, msg, w, l);
    }
}

void drawText(HDC dc, int x, int y, const std::wstring& text, int size = 20, COLORREF color = RGB(210, 222, 214)) {
    HFONT font = CreateFontW(size, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas");
    HFONT old = (HFONT)SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    TextOutW(dc, x, y, text.c_str(), (int)text.size());
    SelectObject(dc, old);
    DeleteObject(font);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"ShadowWardWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    RECT r{0, 0, ScreenW, ScreenH};
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Shadow Ward - tiny C++ fixed-camera horror",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                              r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInst, nullptr);
    HDC dc = GetDC(hwnd);

    Mesh playerMesh = loadObj("assets/player.obj");
    Mesh stalkerMesh = loadObj("assets/stalker.obj");
    Mesh keyMesh = loadObj("assets/key.obj");
    Mesh crateMesh = loadObj("assets/crate.obj");
    Mesh roomFloor = makeBox(18.0f, 0.12f, 26.0f);
    Mesh wallLong = makeBox(0.3f, 3.0f, 26.0f);
    Mesh wallWide = makeBox(18.0f, 3.0f, 0.3f);
    Mesh pillar = makeBox(2.6f, 2.4f, 3.2f);

    Actor player{{-5.8f, 0.0f, -6.0f}, 0.1f, 100, 0.0f};
    std::vector<Actor> enemies = {{{5.8f, 0.0f, 3.8f}, Pi, 45, 0.0f}, {{-5.7f, 0.0f, 12.3f}, 0.0f, 60, 0.0f}};
    std::vector<Item> items = {{{5.7f, 0.15f, -5.9f}, 0, false}, {{-6.5f, 0.15f, 8.6f}, 1, false}, {{6.1f, 0.15f, 13.5f}, 1, false}};
    std::vector<CameraRig> cams = {
        {{-1.0f, 5.0f, -12.0f}, {-4.0f, 0.6f, -3.6f}, 0},
        {{7.8f, 4.6f, 1.8f}, {-1.5f, 0.4f, 3.2f}, 1},
        {{-6.2f, 5.1f, 16.4f}, {1.2f, 0.3f, 10.5f}, 2},
    };

    int ammo = 6;
    bool hasKey = false;
    bool won = false;
    bool dead = false;

    LARGE_INTEGER freq{}, last{};
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    while (g_running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float dt = std::min(0.05f, (float)(now.QuadPart - last.QuadPart) / (float)freq.QuadPart);
        last = now;

        bool pressE = g_keys['E'] && !g_prevKeys['E'];
        bool pressSpace = g_keys[VK_SPACE] && !g_prevKeys[VK_SPACE];
        bool pressR = g_keys['R'] && !g_prevKeys['R'];
        if (g_keys[VK_ESCAPE]) g_running = false;

        if (pressR && (dead || won)) {
            player = {{-5.8f, 0.0f, -6.0f}, 0.1f, 100, 0.0f};
            enemies = {{{5.8f, 0.0f, 3.8f}, Pi, 45, 0.0f}, {{-5.7f, 0.0f, 12.3f}, 0.0f, 60, 0.0f}};
            for (auto& item : items) item.taken = false;
            ammo = 6; hasKey = false; won = false; dead = false;
        }

        if (!dead && !won) {
            float turn = (g_keys['D'] ? 1.0f : 0.0f) - (g_keys['A'] ? 1.0f : 0.0f);
            float move = (g_keys['W'] ? 1.0f : 0.0f) - (g_keys['S'] ? 1.0f : 0.0f);
            player.yaw += turn * dt * 2.3f;
            Vec3 dir{std::sin(player.yaw), 0, std::cos(player.yaw)};
            Vec3 next = player.pos + dir * (move * dt * 3.2f);
            if (!blocked(next)) player.pos = next;

            for (auto& item : items) {
                if (!item.taken && dist2D(player.pos, item.pos) < 1.15f && pressE) {
                    item.taken = true;
                    if (item.type == 0) hasKey = true;
                    if (item.type == 1) ammo += 3;
                }
            }
            if (hasKey && player.pos.z > 14.5f && pressE) won = true;

            if (pressSpace && ammo > 0) {
                ammo--;
                Vec3 aim{std::sin(player.yaw), 0, std::cos(player.yaw)};
                for (auto& e : enemies) {
                    if (e.hp <= 0) continue;
                    Vec3 to = e.pos - player.pos;
                    float d = dist2D(e.pos, player.pos);
                    if (d < 7.0f && dot(norm({to.x, 0, to.z}), aim) > 0.72f) {
                        e.hp -= 30;
                        break;
                    }
                }
            }

            for (auto& e : enemies) {
                if (e.hp <= 0) continue;
                Vec3 to = player.pos - e.pos;
                float d = dist2D(e.pos, player.pos);
                e.yaw = std::atan2(to.x, to.z);
                if (d < 8.5f) {
                    Vec3 step = norm({to.x, 0, to.z}) * (dt * 1.05f);
                    Vec3 np = e.pos + step;
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
        CameraRig cam = *std::find_if(cams.begin(), cams.end(), [z](const CameraRig& c) { return c.zone == z; });
        Vec3 forward = norm(cam.target - cam.pos);
        Vec3 right = norm(cross(forward, {0, 1, 0}));
        Vec3 up = cross(right, forward);

        HDC mem = CreateCompatibleDC(dc);
        HBITMAP bmp = CreateCompatibleBitmap(dc, ScreenW, ScreenH);
        HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);
        HBRUSH bg = CreateSolidBrush(RGB(8, 10, 12));
        RECT full{0, 0, ScreenW, ScreenH};
        FillRect(mem, &full, bg);
        DeleteObject(bg);

        std::vector<DrawTri> draw;
        addMesh(draw, roomFloor, {0, -0.56f, 2.5f}, 0, 1, RGB(58, 61, 58), cam.pos, right, up, forward);
        addMesh(draw, wallLong, {-9.0f, 0.9f, 2.5f}, 0, 1, RGB(51, 55, 61), cam.pos, right, up, forward);
        addMesh(draw, wallLong, {9.0f, 0.9f, 2.5f}, 0, 1, RGB(45, 50, 55), cam.pos, right, up, forward);
        addMesh(draw, wallWide, {0, 0.9f, -10.5f}, 0, 1, RGB(60, 57, 53), cam.pos, right, up, forward);
        addMesh(draw, wallWide, {0, 0.9f, 15.7f}, 0, 1, RGB(60, 48, 48), cam.pos, right, up, forward);
        addMesh(draw, makeBox(3.0f, 2.8f, 0.35f), {-4.4f, 0.9f, 0.0f}, 0, 1, RGB(42, 46, 49), cam.pos, right, up, forward);
        addMesh(draw, makeBox(3.0f, 2.8f, 0.35f), {4.4f, 0.9f, 0.0f}, 0, 1, RGB(42, 46, 49), cam.pos, right, up, forward);
        addMesh(draw, pillar, {3.5f, 0.35f, 6.6f}, 0, 1, RGB(49, 46, 43), cam.pos, right, up, forward);
        addMesh(draw, crateMesh, {-2.4f, 0.0f, -5.2f}, 0.35f, 1.2f, RGB(78, 62, 43), cam.pos, right, up, forward);
        addMesh(draw, crateMesh, {6.8f, 0.0f, 9.7f}, -0.25f, 1.0f, RGB(71, 58, 45), cam.pos, right, up, forward);
        for (auto& item : items) {
            if (!item.taken) addMesh(draw, item.type == 0 ? keyMesh : crateMesh, item.pos, (float)GetTickCount64() * 0.0015f, item.type == 0 ? 1.1f : 0.45f,
                                     item.type == 0 ? RGB(220, 188, 64) : RGB(92, 95, 80), cam.pos, right, up, forward);
        }
        for (auto& e : enemies) {
            if (e.hp > 0) addMesh(draw, stalkerMesh, e.pos, e.yaw, 1.15f, RGB(105, 51, 46), cam.pos, right, up, forward);
        }
        addMesh(draw, playerMesh, player.pos, player.yaw, 1.0f, RGB(69, 116, 126), cam.pos, right, up, forward);

        std::sort(draw.begin(), draw.end(), [](const DrawTri& a, const DrawTri& b) { return a.depth > b.depth; });
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(14, 17, 18));
        HPEN oldPen = (HPEN)SelectObject(mem, pen);
        for (const auto& t : draw) {
            HBRUSH brush = CreateSolidBrush(t.color);
            HBRUSH oldBrush = (HBRUSH)SelectObject(mem, brush);
            Polygon(mem, t.p, 3);
            SelectObject(mem, oldBrush);
            DeleteObject(brush);
        }
        SelectObject(mem, oldPen);
        DeleteObject(pen);

        HBRUSH panel = CreateSolidBrush(RGB(12, 16, 18));
        RECT hud{0, ScreenH - 82, ScreenW, ScreenH};
        FillRect(mem, &hud, panel);
        DeleteObject(panel);
        drawText(mem, 22, ScreenH - 68, L"HP " + std::to_wstring(std::max(0, player.hp)) + L"   AMMO " + std::to_wstring(ammo) +
                 L"   KEY " + std::wstring(hasKey ? L"YES" : L"NO"), 22);
        drawText(mem, 22, ScreenH - 36, L"W/S move  A/D turn  Space fire  E interact", 18, RGB(144, 157, 148));
        if (!hasKey && dist2D(player.pos, items[0].pos) < 1.4f && !items[0].taken) drawText(mem, 340, 28, L"Press E: take ward key", 24, RGB(230, 210, 120));
        if (hasKey && player.pos.z > 14.2f) drawText(mem, 340, 28, L"Press E: unlock service door", 24, RGB(230, 210, 120));
        if (dead) drawText(mem, 365, 228, L"YOU DIED - press R", 34, RGB(210, 70, 62));
        if (won) drawText(mem, 286, 228, L"ESCAPED THE SHADOW WARD - press R", 30, RGB(130, 226, 170));

        BitBlt(dc, 0, 0, ScreenW, ScreenH, mem, 0, 0, SRCCOPY);
        SelectObject(mem, oldBmp);
        DeleteObject(bmp);
        DeleteDC(mem);

        std::copy(std::begin(g_keys), std::end(g_keys), std::begin(g_prevKeys));
        Sleep(1);
    }

    ReleaseDC(hwnd, dc);
    return 0;
}

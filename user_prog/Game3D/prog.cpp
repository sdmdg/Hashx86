/**
 * @file        prog.cpp
 * @brief       3D Game Engine
 *
 * @date        28/01/2026
 * @version     1.0.0
 */

/*
 * 3D Game Engine Module
 *
 * NOTE: The 3D rendering logic and math libraries in this file were generated
 * with assistance from Gemini and Claude to demonstrate user-space capabilities.
 * Therefore, full credit goes to the LLMs :)
 */

#include <Bitmap.h>
#include <Hx86/Hx86.h>
#include <Math3D.h>
#include <Renderer3D.h>

// Scancode defines for WASD
#define SC_W 0x11
#define SC_A 0x1E
#define SC_S 0x1F
#define SC_D 0x20
#define SC_SPACE 0x39
#define SC_LSHIFT 0x2A
#define SC_ESC 0x01
#define SC_Q 0x10
#define SC_E 0x12

// Internal render resolution (half the screen)
#define RENDER_W 800
#define RENDER_H 450

// ============================================================================
// Sky Sphere Generator
// ============================================================================

Mesh* GenerateSkySphere(int stacks, int slices) {
    Mesh* mesh = new Mesh();
    if (!mesh) return nullptr;
    int triangleCount = stacks * slices * 2;
    mesh->tris = new Triangle[triangleCount];
    if (!mesh->tris) {
        delete mesh;
        return nullptr;
    }
    mesh->triCount = 0;

    float radius = 800.0f;

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            float phi0 = (float)i / stacks * PI;
            float phi1 = (float)(i + 1) / stacks * PI;
            float theta0 = (float)j / slices * TWO_PI;
            float theta1 = (float)(j + 1) / slices * TWO_PI;

            Vec3 p0(radius * sin(phi0) * cos(theta0), radius * cos(phi0),
                    radius * sin(phi0) * sin(theta0));
            Vec3 p1(radius * sin(phi1) * cos(theta0), radius * cos(phi1),
                    radius * sin(phi1) * sin(theta0));
            Vec3 p2(radius * sin(phi0) * cos(theta1), radius * cos(phi0),
                    radius * sin(phi0) * sin(theta1));
            Vec3 p3(radius * sin(phi1) * cos(theta1), radius * cos(phi1),
                    radius * sin(phi1) * sin(theta1));

            Vec2 uv0(1.0f - (float)j / slices, (float)i / stacks);
            Vec2 uv1(1.0f - (float)j / slices, (float)(i + 1) / stacks);
            Vec2 uv2(1.0f - (float)(j + 1) / slices, (float)i / stacks);
            Vec2 uv3(1.0f - (float)(j + 1) / slices, (float)(i + 1) / stacks);

            // Triangle 1
            Triangle* t1 = &mesh->tris[mesh->triCount++];
            t1->p[0] = p0;
            t1->p[1] = p1;
            t1->p[2] = p2;
            t1->uv[0] = uv0;
            t1->uv[1] = uv1;
            t1->uv[2] = uv2;
            t1->n[0] = p0.Normalized() * -1.0f;
            t1->n[1] = p1.Normalized() * -1.0f;
            t1->n[2] = p2.Normalized() * -1.0f;

            // Triangle 2
            Triangle* t2 = &mesh->tris[mesh->triCount++];
            t2->p[0] = p2;
            t2->p[1] = p1;
            t2->p[2] = p3;
            t2->uv[0] = uv2;
            t2->uv[1] = uv1;
            t2->uv[2] = uv3;
            t2->n[0] = p2.Normalized() * -1.0f;
            t2->n[1] = p1.Normalized() * -1.0f;
            t2->n[2] = p3.Normalized() * -1.0f;
        }
    }

    return mesh;
}

// ============================================================================
// Helper: Load file from disk via syscall
// ============================================================================

uint8_t* LoadFileData(const char* filename, uint32_t* outSize) {
    // Buffer large enough for textures (~2MB BMP files)
    uint32_t maxSize = 2 * 1024 * 1024 + 4096;
    uint8_t* buffer = new uint8_t[maxSize];
    if (!buffer) {
        printf("Failed to allocate file buffer for %s\n", filename);
        return nullptr;
    }

    uint32_t actualSize = 0;
    int32_t result = syscall_read_file(filename, buffer, maxSize, &actualSize);

    if (result <= 0) {
        printf("Failed to load file: %s\n", filename);
        delete[] buffer;
        return nullptr;
    }

    // IMPORTANT: Use the actual bytes read (result), not the reported file size
    uint32_t bytesRead = (uint32_t)result;
    if (outSize) *outSize = bytesRead;
    printf("Loaded file: %s (%d bytes)\n", filename, bytesRead);
    return buffer;
}

// ============================================================================
// Upscale blit: render buffer (RENDER_W x RENDER_H) → screen (screenW x screenH)
// ============================================================================

static void BlitUpscale(uint32_t* dest, int destW, int destH, uint32_t* src, int srcW, int srcH) {
    for (int y = 0; y < destH; y++) {
        int srcY = (y * srcH) / destH;
        int dstOff = y * destW;
        int srcOff = srcY * srcW;
        for (int x = 0; x < destW; x++) {
            int srcX = (x * srcW) / destW;
            dest[dstOff + x] = src[srcOff + srcX];
        }
    }
}

// ============================================================================
// MAIN GAME
// ============================================================================

extern "C" void _start(void* arg) {
    init_sys(arg);

    printf("[Game3D] Starting 3D Engine...\n");

    // 1. Get framebuffer
    FramebufferInfo fb = syscall_get_framebuffer();
    uint32_t* screenBuffer = (uint32_t*)fb.buffer;
    int screenW = (int)fb.width;
    int screenH = (int)fb.height;
    printf("[Game3D] Framebuffer: %dx%d @ 0x%x\n", screenW, screenH, fb.buffer);

    // Allocate internal render buffer at half resolution
    uint32_t* renderBuffer = new uint32_t[RENDER_W * RENDER_H];
    if (!renderBuffer) {
        printf("[Game3D] FATAL: Failed to allocate render buffer!\n");
        syscall_exit(1);
    }

    // Initialize Renderer at internal resolution
    Renderer3D* renderer = new Renderer3D(RENDER_W, RENDER_H);
    if (!renderer) {
        printf("[Game3D] FATAL: Failed to allocate Renderer3D!\n");
        syscall_exit(1);
    }

    printf("[Game3D] Render at %dx%d, upscale to %dx%d\n", RENDER_W, RENDER_H, screenW, screenH);

    // Generate Sky Sphere
    Mesh* skyMesh = GenerateSkySphere(16, 16);

    // Load Textures from disk
    Bitmap* skyTexture = nullptr;
    Bitmap* stoneTexture = nullptr;

    uint32_t fileSize = 0;
    uint8_t* fileData = nullptr;

    fileData = LoadFileData("ProgFile/Game3D/sky.bmp", &fileSize);
    if (fileData) {
        skyTexture = new Bitmap(fileData, fileSize);
        delete[] fileData;
        if (skyTexture && skyTexture->IsValid()) {
            renderer->SetSkybox(skyTexture);
            printf("[Game3D] Sky texture loaded\n");
        }
    }

    fileData = LoadFileData("ProgFile/Game3D/map.bmp", &fileSize);
    if (fileData) {
        stoneTexture = new Bitmap(fileData, fileSize);
        delete[] fileData;
        printf("[Game3D] Stone texture loaded\n");
    }

    // Load Meshes from disk
    Mesh* wallMesh = nullptr;
    Mesh* floorMesh = nullptr;

    fileData = LoadFileData("ProgFile/Game3D/obj.obj", &fileSize);
    if (fileData) {
        wallMesh = renderer->LoadOBJ(fileData, fileSize);
        delete[] fileData;
        printf("[Game3D] Wall mesh loaded\n");
    }

    fileData = LoadFileData("ProgFile/Game3D/floor.obj", &fileSize);
    if (fileData) {
        floorMesh = renderer->LoadOBJ(fileData, fileSize);
        delete[] fileData;
        printf("[Game3D] Floor mesh loaded\n");
    }

    // Setup 3-Point Lighting
    Light sceneLights[3];
    int activeLightCount = 3;

    sceneLights[0] = Light(Vec3(-0.3f, -1.0f, -0.2f), 0.8f);
    sceneLights[1] = Light(Vec3(0.5f, -0.5f, 0.5f), 0.3f);
    sceneLights[2] = Light(Vec3(0.2f, 0.3f, 1.0f), 0.4f);

    // Camera State
    float camX = 0.0f, camY = 5.0f, camZ = -10.0f;
    float camYaw = 0.0f, camPitch = 0.0f;

    // Input state
    InputState input;

    printf("[Game3D] Entering main loop...\n");

    // ========================================================================
    // MAIN GAME LOOP
    // ========================================================================
    while (1) {
        // POLL INPUT
        syscall_get_input(&input);

        // Check ESC to exit
        if (input.keyStates[SC_ESC]) {
            printf("[Game3D] ESC pressed, exiting...\n");
            syscall_exit(0);
        }

        // CAMERA MOVEMENT
        float speed = 0.4f;
        float yaw = -camYaw;
        float forwardX = sin(yaw);
        float forwardZ = cos(yaw);
        float rightX = cos(yaw);
        float rightZ = -sin(yaw);

        if (input.keyStates[SC_W]) {
            camX += forwardX * speed;
            camZ += forwardZ * speed;
        }
        if (input.keyStates[SC_S]) {
            camX -= forwardX * speed;
            camZ -= forwardZ * speed;
        }
        if (input.keyStates[SC_A]) {
            camX -= rightX * speed;
            camZ -= rightZ * speed;
        }
        if (input.keyStates[SC_D]) {
            camX += rightX * speed;
            camZ += rightZ * speed;
        }
        if (input.keyStates[SC_SPACE]) camY += speed;
        if (input.keyStates[SC_LSHIFT]) camY -= speed;

        // MOUSE LOOK
        float sensitivity = 0.005f;
        camYaw -= (float)input.mouseDX * sensitivity;
        camPitch += (float)input.mouseDY * sensitivity;
        if (camPitch > 1.5f) camPitch = 1.5f;
        if (camPitch < -1.5f) camPitch = -1.5f;

        // RENDER TO INTERNAL BUFFER
        renderer->Clear(renderBuffer, 0xFF87CEEB);

        // Draw sky
        if (skyMesh && skyTexture) {
            renderer->SetMaterial(1.0f, 0.0f, 0.0f);
            renderer->BindTexture(skyTexture);
            renderer->DrawMesh(renderBuffer, skyMesh, camX, camY, camZ, camYaw, camPitch,
                               sceneLights, 0);
        }

        // Draw floor
        if (floorMesh) {
            renderer->SetMaterial(0.25f, 0.2f, 16.0f);
            renderer->BindTexture(stoneTexture);
            renderer->DrawMesh(renderBuffer, floorMesh, camX, camY, camZ, camYaw, camPitch,
                               sceneLights, activeLightCount);
        }

        // Draw walls
        if (wallMesh) {
            renderer->SetMaterial(0.2f, 0.3f, 8.0f);
            renderer->BindTexture(stoneTexture);
            renderer->DrawMesh(renderBuffer, wallMesh, camX, camY, camZ, camYaw, camPitch,
                               sceneLights, activeLightCount);
        }

        // Draw crosshair on render buffer
        int cx = RENDER_W / 2;
        int cy = RENDER_H / 2;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int px = cx + dx;
                int py = cy + dy;
                if (px >= 0 && px < RENDER_W && py >= 0 && py < RENDER_H) {
                    renderBuffer[py * RENDER_W + px] = 0xFF00FF00;
                }
            }
        }

        // UPSCALE TO SCREEN
        BlitUpscale(screenBuffer, screenW, screenH, renderBuffer, RENDER_W, RENDER_H);

        // Frame delay (~60fps)
        syscall_sleep(16);
    }
}

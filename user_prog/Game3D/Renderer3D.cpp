/**
 * @file        Renderer3D.cpp
 * @brief       User-space 3D Rendering Engine
 *
 * @date        28/01/2026
 * @version     2.0.0
 */

/*
 * 3D Game Engine Module
 *
 * NOTE: The 3D rendering logic and math libraries in this file were generated
 * with assistance from Gemini and Claude to demonstrate user-space capabilities.
 * Therefore, full credit goes to the LLMs :)
 */

#include <Hx86/debug.h>
#include <Hx86/memory.h>
#include <Renderer3D.h>

// Constants
#define NEAR_PLANE 0.1f
#define FAR_PLANE 1000.0f
#define FOV_FACTOR 800.0f

Renderer3D::Renderer3D(int w, int h) {
    this->width = w;
    this->height = h;
    this->zBuffer = new float[w * h];
    this->halfWidth = w / 2.0f;
    this->halfHeight = h / 2.0f;

    this->enableBackfaceCulling = true;
    this->enableTexturing = true;
    this->skybox = nullptr;
    this->currentTexture = nullptr;

    // Default Material
    this->ambientStrength = 0.2f;
    this->specularStrength = 0.5f;
    this->shininess = 32.0f;

    this->enableZRead = true;
    this->enableZWrite = true;
    this->enableLighting = true;

    // Shadow map
    this->shadowMap = new float[SHADOW_MAP_SIZE * SHADOW_MAP_SIZE];
    this->shadowsEnabled = false;
    this->shadowOrthoSize = 50.0f;
    this->shadowNear = 1.0f;
    this->shadowFar = 200.0f;
    this->shadowLightDir = Vec3(0, -1, 0);

    // Skybox camera
    this->skyYaw = 0.0f;
    this->skyPitch = 0.0f;
}

Renderer3D::~Renderer3D() {
    delete[] this->zBuffer;
    delete[] this->shadowMap;
}

void Renderer3D::Clear(uint32_t* buffer, uint32_t color) {
    int size = width * height;
    for (int i = 0; i < size; i++) zBuffer[i] = 0.0f;

    if (skybox && skybox->IsValid()) {
        int skyW = skybox->GetWidth();
        int skyH = skybox->GetHeight();
        uint32_t* skyData = skybox->GetBuffer();

        for (int y = 0; y < height; y++) {
            int srcY = (y * skyH) / height;
            int yOffset = y * width;
            int skyYOffset = srcY * skyW;

            for (int x = 0; x < width; x++) {
                int srcX = (x * skyW) / width;
                buffer[yOffset + x] = skyData[skyYOffset + srcX];
            }
        }
    } else {
        for (int i = 0; i < size; i++) buffer[i] = color;
    }
}

// Camera-aware panoramic skybox render
void Renderer3D::ClearSky(uint32_t* buffer, float camYaw, float camPitch) {
    int size = width * height;
    for (int i = 0; i < size; i++) zBuffer[i] = 0.0f;

    if (!skybox || !skybox->IsValid()) {
        // Gradient sky fallback
        for (int y = 0; y < height; y++) {
            float t = (float)y / (float)height;
            // Blend from deep blue (top) to light cyan (horizon) to warm (bottom)
            int r, g, b;
            if (t < 0.5f) {
                float s = t * 2.0f;
                r = (int)(30 + s * 105);
                g = (int)(60 + s * 150);
                b = (int)(180 + s * 55);
            } else {
                float s = (t - 0.5f) * 2.0f;
                r = (int)(135 + s * 60);
                g = (int)(210 - s * 40);
                b = (int)(235 - s * 80);
            }
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            uint32_t skyColor = 0xFF000000 | (r << 16) | (g << 8) | b;
            int yOff = y * width;
            for (int x = 0; x < width; x++) {
                buffer[yOff + x] = skyColor;
            }
        }
        return;
    }

    int skyW = skybox->GetWidth();
    int skyH = skybox->GetHeight();
    uint32_t* skyData = skybox->GetBuffer();

    // Simple UV offset approach: scroll horizontally by yaw, offset vertically by pitch
    // This is MUCH faster than per-pixel trig (no sin/cos/atan2 per pixel)
    // Yaw maps to horizontal scroll (full 2*PI = full texture width)
    float yawNorm = camYaw / TWO_PI;  // Normalize to [0, 1) range
    int xOffset = (int)(yawNorm * skyW);
    // Keep positive
    xOffset = ((xOffset % skyW) + skyW) % skyW;

    // Pitch maps to vertical offset (clamp to reasonable range)
    float pitchNorm = camPitch / PI;  // -0.5 to 0.5
    int yOffset = (int)(pitchNorm * skyH * 0.5f);

    for (int y = 0; y < height; y++) {
        int srcY = (y * skyH) / height + yOffset;
        // Clamp vertically (don't wrap sky)
        if (srcY < 0) srcY = 0;
        if (srcY >= skyH) srcY = skyH - 1;

        int dstOff = y * width;
        int srcRowOff = srcY * skyW;

        for (int x = 0; x < width; x++) {
            int srcX = ((x * skyW) / width + xOffset) % skyW;
            buffer[dstOff + x] = skyData[srcRowOff + srcX];
        }
    }
}

void Renderer3D::BindTexture(Bitmap* tex) {
    this->currentTexture = tex;
    if (tex && tex->IsValid()) {
        this->texWidthMask = tex->GetWidth() - 1;
        this->texHeightMask = tex->GetHeight() - 1;
    }
}

void Renderer3D::SetSkybox(Bitmap* sky) {
    this->skybox = sky;
}

void Renderer3D::SetMaterial(float a, float s, float sh) {
    ambientStrength = a;
    specularStrength = s;
    shininess = sh;
}

// ============================================================================
// LIGHTING
// ============================================================================

float Renderer3D::CalculateLighting(const Vec3& normal, const Vec3& viewDir, Light* lights,
                                    int lightCount) {
    float total = ambientStrength;

    for (int i = 0; i < lightCount; i++) {
        // Diffuse (Lambert)
        float ndotl = normal.Dot(lights[i].direction * -1.0f);
        if (ndotl < 0.0f) ndotl = 0.0f;
        total += ndotl * lights[i].intensity;

        // Specular (Blinn-Phong)
        if (specularStrength > 0.0f && ndotl > 0.0f) {
            Vec3 halfDir = (viewDir + lights[i].direction * -1.0f).Normalized();
            float spec = normal.Dot(halfDir);
            if (spec < 0.0f) spec = 0.0f;
            // Approximate pow with multiplications
            float specPow = spec;
            int shinInt = (int)shininess;
            for (int s = 1; s < shinInt && s < 32; s *= 2) {
                specPow *= specPow;
            }
            total += specPow * specularStrength * lights[i].intensity;
        }
    }

    if (total > 1.5f) total = 1.5f;
    return total;
}

// ============================================================================
// SHADOW MAP
// ============================================================================

void Renderer3D::SetupShadows(const Vec3& lightDir, float orthoSize, float nearPlane,
                              float farPlane) {
    shadowLightDir = lightDir;
    shadowLightDir.Normalize();
    shadowOrthoSize = orthoSize;
    shadowNear = nearPlane;
    shadowFar = farPlane;
    shadowsEnabled = true;
}

void Renderer3D::BeginShadowPass(float centerX, float centerY, float centerZ) {
    // Clear shadow map
    int smSize = SHADOW_MAP_SIZE * SHADOW_MAP_SIZE;
    for (int i = 0; i < smSize; i++) {
        shadowMap[i] = 1e30f;  // Far away
    }

    // Cache the light-space transform center
    slCenterX = centerX;
    slCenterY = centerY;
    slCenterZ = centerZ;

    // Build light view rotation
    // Look along the light direction
    // Light direction = direction light is shining (e.g. (0, -1, 0) = straight down)
    Vec3 lightForward = shadowLightDir;
    lightForward.Normalize();

    // Compute yaw/pitch from light direction
    float yaw = atan2(lightForward.x, lightForward.z);
    float pitch = asin(-lightForward.y);

    slCosY = cos(yaw);
    slSinY = sin(yaw);
    slCosP = cos(pitch);
    slSinP = sin(pitch);
}

Vec3 Renderer3D::WorldToShadowUV(const Vec3& worldPos) {
    // Translate to shadow center
    float tx = worldPos.x - slCenterX;
    float ty = worldPos.y - slCenterY;
    float tz = worldPos.z - slCenterZ;

    // Rotate by yaw (Y-axis)
    float rx = tx * slCosY - tz * slSinY;
    float rz = tx * slSinY + tz * slCosY;

    // Rotate by pitch (X-axis)
    float ry = ty * slCosP - rz * slSinP;
    float rz2 = ty * slSinP + rz * slCosP;

    // Orthographic projection to [0,1]
    float invSize = 0.5f / shadowOrthoSize;
    float u = rx * invSize + 0.5f;
    float v = ry * invSize + 0.5f;

    return Vec3(u, v, -rz2);  // z = depth in light space (negated so farther from light = larger)
}

void Renderer3D::RasterizeShadowTriangle(Vec3 p0, Vec3 p1, Vec3 p2) {
    // Convert world positions to shadow UV + depth
    Vec3 s0 = WorldToShadowUV(p0);
    Vec3 s1 = WorldToShadowUV(p1);
    Vec3 s2 = WorldToShadowUV(p2);

    // Convert to shadow map pixel coordinates
    float sm = (float)SHADOW_MAP_SIZE;
    s0.x *= sm;
    s0.y *= sm;
    s1.x *= sm;
    s1.y *= sm;
    s2.x *= sm;
    s2.y *= sm;

    // Sort by Y
    if (s0.y > s1.y) {
        Vec3 tmp = s0;
        s0 = s1;
        s1 = tmp;
    }
    if (s0.y > s2.y) {
        Vec3 tmp = s0;
        s0 = s2;
        s2 = tmp;
    }
    if (s1.y > s2.y) {
        Vec3 tmp = s1;
        s1 = s2;
        s2 = tmp;
    }

    int y0 = (int)ceilf(s0.y);
    int y1 = (int)ceilf(s1.y);
    int y2 = (int)ceilf(s2.y);

    if (y0 == y2) return;
    if (y0 < 0) y0 = 0;
    if (y2 > SHADOW_MAP_SIZE) y2 = SHADOW_MAP_SIZE;

    float dy02 = s2.y - s0.y;
    if (dy02 == 0.0f) return;
    float invDy02 = 1.0f / dy02;

    for (int y = y0; y < y2; y++) {
        if (y < 0 || y >= SHADOW_MAP_SIZE) continue;

        float t02 = ((float)y - s0.y) * invDy02;
        float xA = s0.x + (s2.x - s0.x) * t02;
        float zA = s0.z + (s2.z - s0.z) * t02;

        float xB, zB;
        if (y < y1) {
            float dy01 = s1.y - s0.y;
            if (dy01 == 0.0f) continue;
            float t01 = ((float)y - s0.y) / dy01;
            xB = s0.x + (s1.x - s0.x) * t01;
            zB = s0.z + (s1.z - s0.z) * t01;
        } else {
            float dy12 = s2.y - s1.y;
            if (dy12 == 0.0f) continue;
            float t12 = ((float)y - s1.y) / dy12;
            xB = s1.x + (s2.x - s1.x) * t12;
            zB = s1.z + (s2.z - s1.z) * t12;
        }

        if (xA > xB) {
            float tmp;
            tmp = xA;
            xA = xB;
            xB = tmp;
            tmp = zA;
            zA = zB;
            zB = tmp;
        }

        int xStart = (int)ceilf(xA);
        int xEnd = (int)ceilf(xB);
        if (xStart < 0) xStart = 0;
        if (xEnd > SHADOW_MAP_SIZE) xEnd = SHADOW_MAP_SIZE;

        float dxAB = xB - xA;
        if (dxAB == 0.0f) continue;
        float invDxAB = 1.0f / dxAB;

        int yOff = y * SHADOW_MAP_SIZE;
        float zStep = (zB - zA) * invDxAB;
        float z = zA + ((float)xStart - xA) * zStep;

        for (int x = xStart; x < xEnd; x++) {
            int idx = yOff + x;
            if (z < shadowMap[idx]) {
                shadowMap[idx] = z;
            }
            z += zStep;
        }
    }
}

void Renderer3D::RenderMeshToShadowMap(Mesh* mesh) {
    if (!mesh || mesh->triCount == 0) return;

    for (int t = 0; t < mesh->triCount; t++) {
        Triangle& tri = mesh->tris[t];
        RasterizeShadowTriangle(tri.p[0], tri.p[1], tri.p[2]);
    }
}

void Renderer3D::EndShadowPass() {
    // Shadow map is complete - nothing to finalize
}

float Renderer3D::SampleShadow(const Vec3& worldPos) {
    if (!shadowsEnabled) return 1.0f;

    Vec3 shadowUV = WorldToShadowUV(worldPos);

    // Out of shadow map bounds = lit
    if (shadowUV.x < 0.01f || shadowUV.x > 0.99f || shadowUV.y < 0.01f || shadowUV.y > 0.99f) {
        return 1.0f;
    }

    float currentDepth = shadowUV.z;

    // 3x3 PCF (Percentage Closer Filtering) for smooth shadow edges
    float fx = shadowUV.x * (SHADOW_MAP_SIZE - 1);
    float fy = shadowUV.y * (SHADOW_MAP_SIZE - 1);
    int ix = (int)fx;
    int iy = (int)fy;

    // Clamp to valid range (leaving room for -1 and +1 neighbors)
    if (ix < 1) ix = 1;
    if (ix >= SHADOW_MAP_SIZE - 1) ix = SHADOW_MAP_SIZE - 2;
    if (iy < 1) iy = 1;
    if (iy >= SHADOW_MAP_SIZE - 1) iy = SHADOW_MAP_SIZE - 2;

    // Sample 9 neighboring texels (3x3 kernel) and count lit samples
    float litCount = 0.0f;
    for (int dy = -1; dy <= 1; dy++) {
        int sy = iy + dy;
        int rowOff = sy * SHADOW_MAP_SIZE;
        for (int dx = -1; dx <= 1; dx++) {
            float storedDepth = shadowMap[rowOff + ix + dx];
            if (currentDepth <= storedDepth + SHADOW_BIAS) {
                litCount += 1.0f;
            }
        }
    }

    // Normalize: 0 = fully shadowed, 9 = fully lit
    float shadowFactor = litCount * (1.0f / 9.0f);

    // Map to shadow intensity: 0.3 (full shadow) to 1.0 (full light)
    return 0.3f + shadowFactor * 0.7f;
}

// ============================================================================
// MESH DRAWING
// ============================================================================

void Renderer3D::DrawMesh(uint32_t* buffer, Mesh* mesh, float camX, float camY, float camZ,
                          float camYaw, float camPitch, Light* lights, int lightCount) {
    if (!mesh || mesh->triCount == 0) return;

    // Build view matrix
    float yaw = -camYaw;
    float cosY = cos(yaw), sinY = sin(yaw);
    float cosP = cos(camPitch), sinP = sin(camPitch);

    // Forward direction
    Vec3 viewDir(sinY * cosP, -sinP, cosY * cosP);
    viewDir.Normalize();

    for (int t = 0; t < mesh->triCount; t++) {
        Triangle& tri = mesh->tris[t];

        // Transform vertices to camera space
        Vertex v[3];
        for (int i = 0; i < 3; i++) {
            // Store world position for shadow lookup
            v[i].worldPos = tri.p[i];

            // Translate relative to camera
            float tx = tri.p[i].x - camX;
            float ty = tri.p[i].y - camY;
            float tz = tri.p[i].z - camZ;

            // Rotate by yaw (Y-axis)
            float rx = tx * cosY - tz * sinY;
            float rz = tx * sinY + tz * cosY;

            // Rotate by pitch (X-axis)
            float ry = ty * cosP - rz * sinP;
            float rz2 = ty * sinP + rz * cosP;

            v[i].pos = Vec3(rx, ry, rz2);
            v[i].uv = tri.uv[i];

            // Transform normal
            float nx = tri.n[i].x * cosY - tri.n[i].z * sinY;
            float nz = tri.n[i].x * sinY + tri.n[i].z * cosY;
            float ny = tri.n[i].y * cosP - nz * sinP;
            float nz2 = tri.n[i].y * sinP + nz * cosP;
            v[i].normal = Vec3(nx, ny, nz2);

            // Per-vertex lighting
            v[i].light = CalculateLighting(tri.n[i], viewDir, lights, lightCount);
        }

        // Backface culling in camera space
        if (enableBackfaceCulling) {
            Vec3 e1 = v[1].pos - v[0].pos;
            Vec3 e2 = v[2].pos - v[0].pos;
            Vec3 faceNormal = e1.Cross(e2);
            if (faceNormal.Dot(v[0].pos) >= 0.0f) continue;
        }

        // Clip and project
        ClipTriangle(v[0], v[1], v[2], buffer);
    }
}

// ============================================================================
// CLIPPING (Near plane)
// ============================================================================

static Vertex LerpVertex(const Vertex& a, const Vertex& b, float t) {
    Vertex result;
    result.pos = Vec3::Lerp(a.pos, b.pos, t);
    result.uv = Vec2::Lerp(a.uv, b.uv, t);
    result.normal = Vec3::Lerp(a.normal, b.normal, t);
    result.worldPos = Vec3::Lerp(a.worldPos, b.worldPos, t);
    result.light = a.light + (b.light - a.light) * t;
    return result;
}

void Renderer3D::ClipTriangle(Vertex v1, Vertex v2, Vertex v3, uint32_t* buffer) {
    // Near plane clipping (z > NEAR_PLANE in camera space)
    Vertex verts[3] = {v1, v2, v3};
    Vertex clipped[4];
    int clipCount = 0;

    for (int i = 0; i < 3; i++) {
        Vertex& curr = verts[i];
        Vertex& next = verts[(i + 1) % 3];

        bool currInside = curr.pos.z > NEAR_PLANE;
        bool nextInside = next.pos.z > NEAR_PLANE;

        if (currInside) {
            if (clipCount < 4) clipped[clipCount++] = curr;
        }

        if (currInside != nextInside) {
            // Compute intersection
            float t = (NEAR_PLANE - curr.pos.z) / (next.pos.z - curr.pos.z);
            if (clipCount < 4) clipped[clipCount++] = LerpVertex(curr, next, t);
        }
    }

    // Project and rasterize the clipped polygon
    if (clipCount < 3) return;

    for (int i = 0; i < clipCount; i++) {
        // Perspective projection
        float invZ = 1.0f / clipped[i].pos.z;
        clipped[i].pos.x = clipped[i].pos.x * FOV_FACTOR * invZ + halfWidth;
        clipped[i].pos.y = -clipped[i].pos.y * FOV_FACTOR * invZ + halfHeight;

        // Store 1/z for z-buffer AND perspective-correct interpolation
        // Pre-divide UV and lighting by Z for perspective-correct interpolation
        clipped[i].uv.x *= invZ;
        clipped[i].uv.y *= invZ;
        clipped[i].light *= invZ;
        clipped[i].worldPos = clipped[i].worldPos * invZ;
        clipped[i].pos.z = invZ;  // Store 1/z
    }

    FillTriangle(buffer, clipped[0], clipped[1], clipped[2]);

    if (clipCount == 4) {
        FillTriangle(buffer, clipped[0], clipped[2], clipped[3]);
    }
}

// ============================================================================
// TRIANGLE RASTERIZER
// Perspective-correct texturing + optimized scanline with delta-stepping
// ============================================================================

void Renderer3D::FillTriangle(uint32_t* buffer, Vertex v1, Vertex v2, Vertex v3) {
    // Sort vertices by Y (top to bottom)
    if (v1.pos.y > v2.pos.y) {
        Vertex tmp = v1;
        v1 = v2;
        v2 = tmp;
    }
    if (v1.pos.y > v3.pos.y) {
        Vertex tmp = v1;
        v1 = v3;
        v3 = tmp;
    }
    if (v2.pos.y > v3.pos.y) {
        Vertex tmp = v2;
        v2 = v3;
        v3 = tmp;
    }

    int y1 = (int)ceilf(v1.pos.y);
    int y2 = (int)ceilf(v2.pos.y);
    int y3 = (int)ceilf(v3.pos.y);

    if (y1 == y3) return;  // Degenerate

    // Clamp to screen
    if (y1 < 0) y1 = 0;
    if (y3 > height) y3 = height;

    float dy13 = v3.pos.y - v1.pos.y;
    if (dy13 == 0.0f) return;
    float invDy13 = 1.0f / dy13;

    bool hasTexture = enableTexturing && currentTexture && currentTexture->IsValid();
    uint32_t* texData = hasTexture ? currentTexture->GetBuffer() : nullptr;
    int texW = hasTexture ? currentTexture->GetWidth() : 0;
    int texH = hasTexture ? currentTexture->GetHeight() : 0;

    // NOTE: At this point, uv, light, and worldPos are already pre-divided by Z
    // in ClipTriangle (perspective-correct setup). pos.z stores 1/Z.

    for (int y = y1; y < y3; y++) {
        if (y < 0 || y >= height) continue;

        // Interpolate along long edge (v1 -> v3)
        float t13 = ((float)y - v1.pos.y) * invDy13;

        float xA = v1.pos.x + (v3.pos.x - v1.pos.x) * t13;
        float zA = v1.pos.z + (v3.pos.z - v1.pos.z) * t13;    // 1/z
        float uozA = v1.uv.x + (v3.uv.x - v1.uv.x) * t13;     // u/z
        float vozA = v1.uv.y + (v3.uv.y - v1.uv.y) * t13;     // v/z
        float lozA = v1.light + (v3.light - v1.light) * t13;  // light/z

        // World pos / z for shadow lookup
        float wxozA = v1.worldPos.x + (v3.worldPos.x - v1.worldPos.x) * t13;
        float wyozA = v1.worldPos.y + (v3.worldPos.y - v1.worldPos.y) * t13;
        float wzozA = v1.worldPos.z + (v3.worldPos.z - v1.worldPos.z) * t13;

        // Interpolate along short edge
        float xB, zB, uozB, vozB, lozB;
        float wxozB, wyozB, wzozB;

        if (y < y2) {
            float dy12 = v2.pos.y - v1.pos.y;
            if (dy12 == 0.0f) continue;
            float t12 = ((float)y - v1.pos.y) / dy12;
            xB = v1.pos.x + (v2.pos.x - v1.pos.x) * t12;
            zB = v1.pos.z + (v2.pos.z - v1.pos.z) * t12;
            uozB = v1.uv.x + (v2.uv.x - v1.uv.x) * t12;
            vozB = v1.uv.y + (v2.uv.y - v1.uv.y) * t12;
            lozB = v1.light + (v2.light - v1.light) * t12;
            wxozB = v1.worldPos.x + (v2.worldPos.x - v1.worldPos.x) * t12;
            wyozB = v1.worldPos.y + (v2.worldPos.y - v1.worldPos.y) * t12;
            wzozB = v1.worldPos.z + (v2.worldPos.z - v1.worldPos.z) * t12;
        } else {
            float dy23 = v3.pos.y - v2.pos.y;
            if (dy23 == 0.0f) continue;
            float t23 = ((float)y - v2.pos.y) / dy23;
            xB = v2.pos.x + (v3.pos.x - v2.pos.x) * t23;
            zB = v2.pos.z + (v3.pos.z - v2.pos.z) * t23;
            uozB = v2.uv.x + (v3.uv.x - v2.uv.x) * t23;
            vozB = v2.uv.y + (v3.uv.y - v2.uv.y) * t23;
            lozB = v2.light + (v3.light - v2.light) * t23;
            wxozB = v2.worldPos.x + (v3.worldPos.x - v2.worldPos.x) * t23;
            wyozB = v2.worldPos.y + (v3.worldPos.y - v2.worldPos.y) * t23;
            wzozB = v2.worldPos.z + (v3.worldPos.z - v2.worldPos.z) * t23;
        }

        // Ensure left < right
        if (xA > xB) {
            float tmp;
            tmp = xA;
            xA = xB;
            xB = tmp;
            tmp = zA;
            zA = zB;
            zB = tmp;
            tmp = uozA;
            uozA = uozB;
            uozB = tmp;
            tmp = vozA;
            vozA = vozB;
            vozB = tmp;
            tmp = lozA;
            lozA = lozB;
            lozB = tmp;
            tmp = wxozA;
            wxozA = wxozB;
            wxozB = tmp;
            tmp = wyozA;
            wyozA = wyozB;
            wyozB = tmp;
            tmp = wzozA;
            wzozA = wzozB;
            wzozB = tmp;
        }

        int xStart = (int)ceilf(xA);
        int xEnd = (int)ceilf(xB);
        if (xStart < 0) xStart = 0;
        if (xEnd > width) xEnd = width;

        float dxAB = xB - xA;
        if (dxAB == 0.0f) continue;
        float invDxAB = 1.0f / dxAB;

        // Pre-compute per-scanline deltas (delta-stepping optimization)
        float dz = (zB - zA) * invDxAB;
        float duoz = (uozB - uozA) * invDxAB;
        float dvoz = (vozB - vozA) * invDxAB;
        float dloz = (lozB - lozA) * invDxAB;
        float dwxoz = (wxozB - wxozA) * invDxAB;
        float dwyoz = (wyozB - wyozA) * invDxAB;
        float dwzoz = (wzozB - wzozA) * invDxAB;

        // Starting values (sub-pixel correct)
        float startOffset = (float)xStart - xA;
        float z = zA + dz * startOffset;
        float uoz = uozA + duoz * startOffset;
        float voz = vozA + dvoz * startOffset;
        float loz = lozA + dloz * startOffset;
        float wxoz = wxozA + dwxoz * startOffset;
        float wyoz = wyozA + dwyoz * startOffset;
        float wzoz = wzozA + dwzoz * startOffset;

        int yOffset = y * width;

        for (int x = xStart; x < xEnd; x++) {
            int idx = yOffset + x;

            // Z-buffer test (higher 1/z = closer)
            if (z <= zBuffer[idx]) {
                z += dz;
                uoz += duoz;
                voz += dvoz;
                loz += dloz;
                wxoz += dwxoz;
                wyoz += dwyoz;
                wzoz += dwzoz;
                continue;
            }
            zBuffer[idx] = z;

            // Perspective-correct recovery: divide by 1/z to get actual value
            float realZ = 1.0f / z;
            float u = uoz * realZ;
            float v = voz * realZ;
            float light = loz * realZ;

            uint32_t color;
            if (hasTexture) {
                // Sample texture with wrapping
                int texX = (int)(u * texW) & (texW - 1);
                int texY = (int)(v * texH) & (texH - 1);
                if (texX < 0) texX += texW;
                if (texY < 0) texY += texH;

                color = texData[texY * texW + texX];
            } else {
                color = 0xFFCCCCCC;  // Default grey
            }

            // Shadow test
            if (shadowsEnabled) {
                // Recover world position
                Vec3 wp(wxoz * realZ, wyoz * realZ, wzoz * realZ);
                float shadow = SampleShadow(wp);
                light *= shadow;
            }

            // Apply lighting
            if (light != 1.0f) {
                uint8_t r = (color >> 16) & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = color & 0xFF;

                int ri = (int)(r * light);
                int gi = (int)(g * light);
                int bi = (int)(b * light);
                if (ri > 255) ri = 255;
                if (gi > 255) gi = 255;
                if (bi > 255) bi = 255;

                color = 0xFF000000 | (ri << 16) | (gi << 8) | bi;
            }

            buffer[idx] = color;

            // Step deltas
            z += dz;
            uoz += duoz;
            voz += dvoz;
            loz += dloz;
            wxoz += dwxoz;
            wyoz += dwyoz;
            wzoz += dwzoz;
        }
    }
}

// ============================================================================
// OBJ LOADER (from raw memory buffer)
// ============================================================================

void Renderer3D::SkipWhitespace(char*& ptr) {
    while (*ptr == ' ' || *ptr == '\t') ptr++;
}

float Renderer3D::ParseFloat(char*& ptr) {
    SkipWhitespace(ptr);
    float result = 0.0f;
    float sign = 1.0f;
    if (*ptr == '-') {
        sign = -1.0f;
        ptr++;
    } else if (*ptr == '+')
        ptr++;

    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10.0f + (*ptr - '0');
        ptr++;
    }
    if (*ptr == '.') {
        ptr++;
        float frac = 0.1f;
        while (*ptr >= '0' && *ptr <= '9') {
            result += (*ptr - '0') * frac;
            frac *= 0.1f;
            ptr++;
        }
    }
    return result * sign;
}

int Renderer3D::ParseInt(char*& ptr) {
    SkipWhitespace(ptr);
    int result = 0;
    int sign = 1;
    if (*ptr == '-') {
        sign = -1;
        ptr++;
    }
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }
    return result * sign;
}

Mesh* Renderer3D::LoadOBJ(uint8_t* data, uint32_t dataSize) {
    if (!data || dataSize == 0) return nullptr;

    // First pass: count vertices, normals, UVs, and faces
    int vertCount = 0, uvCount = 0, normCount = 0, faceCount = 0;

    char* ptr = (char*)data;
    char* end = ptr + dataSize;

    while (ptr < end) {
        if (*ptr == 'v') {
            ptr++;
            if (*ptr == ' ' || *ptr == '\t')
                vertCount++;
            else if (*ptr == 't')
                uvCount++;
            else if (*ptr == 'n')
                normCount++;
        } else if (*ptr == 'f' && (*(ptr + 1) == ' ' || *(ptr + 1) == '\t')) {
            faceCount++;
        }
        // Skip to next line
        while (ptr < end && *ptr != '\n') ptr++;
        if (ptr < end) ptr++;
    }

    if (vertCount == 0 || faceCount == 0) return nullptr;

    printf("OBJ: %d verts, %d uvs, %d normals, %d faces\n", vertCount, uvCount, normCount,
           faceCount);

    // Allocate temporary arrays
    Vec3* verts = new Vec3[vertCount + 1];
    Vec2* uvs = uvCount > 0 ? new Vec2[uvCount + 1] : nullptr;
    Vec3* norms = normCount > 0 ? new Vec3[normCount + 1] : nullptr;

    if (!verts) {
        printf("OBJ: Failed to allocate vertex buffer!\n");
        return nullptr;
    }

    // Allocate mesh (overestimate: each face could be a quad = 2 triangles)
    Mesh* mesh = new Mesh();
    if (!mesh) {
        delete[] verts;
        if (uvs) delete[] uvs;
        if (norms) delete[] norms;
        printf("OBJ: Failed to allocate mesh!\n");
        return nullptr;
    }
    mesh->tris = new Triangle[faceCount * 2];
    if (!mesh->tris) {
        delete[] verts;
        if (uvs) delete[] uvs;
        if (norms) delete[] norms;
        delete mesh;
        printf("OBJ: Failed to allocate %d triangles!\n", faceCount * 2);
        return nullptr;
    }
    mesh->triCount = 0;

    // Second pass: parse data
    ptr = (char*)data;
    int vi = 1, ui = 1, ni = 1;

    while (ptr < end) {
        if (*ptr == 'v') {
            ptr++;
            if (*ptr == ' ' || *ptr == '\t') {
                // Vertex position
                float x = ParseFloat(ptr);
                float y = ParseFloat(ptr);
                float z = ParseFloat(ptr);
                verts[vi++] = Vec3(x, y, z);
            } else if (*ptr == 't') {
                ptr++;
                float u = ParseFloat(ptr);
                float v = ParseFloat(ptr);
                if (uvs) uvs[ui++] = Vec2(u, v);
            } else if (*ptr == 'n') {
                ptr++;
                float x = ParseFloat(ptr);
                float y = ParseFloat(ptr);
                float z = ParseFloat(ptr);
                if (norms) norms[ni++] = Vec3(x, y, z);
            }
        } else if (*ptr == 'f' && (*(ptr + 1) == ' ' || *(ptr + 1) == '\t')) {
            ptr++;
            // Parse face indices (v/vt/vn format)
            int faceVerts[4] = {0, 0, 0, 0};
            int faceUVs[4] = {0, 0, 0, 0};
            int faceNorms[4] = {0, 0, 0, 0};
            int faceVertCount = 0;

            while (faceVertCount < 4) {
                SkipWhitespace(ptr);
                if (ptr >= end || *ptr == '\n' || *ptr == '\r') break;

                int v_idx = ParseInt(ptr);
                int vt_idx = 0, vn_idx = 0;

                if (*ptr == '/') {
                    ptr++;
                    if (*ptr != '/') {
                        vt_idx = ParseInt(ptr);
                    }
                    if (*ptr == '/') {
                        ptr++;
                        vn_idx = ParseInt(ptr);
                    }
                }

                faceVerts[faceVertCount] = v_idx;
                faceUVs[faceVertCount] = vt_idx;
                faceNorms[faceVertCount] = vn_idx;
                faceVertCount++;
            }

            if (faceVertCount >= 3) {
                // Triangle 1
                Triangle* t = &mesh->tris[mesh->triCount++];
                for (int i = 0; i < 3; i++) {
                    int idx = (i == 0) ? 0 : (i == 1) ? 1 : 2;
                    if (faceVerts[idx] > 0 && faceVerts[idx] < vi) t->p[i] = verts[faceVerts[idx]];
                    if (uvs && faceUVs[idx] > 0 && faceUVs[idx] < ui) t->uv[i] = uvs[faceUVs[idx]];
                    if (norms && faceNorms[idx] > 0 && faceNorms[idx] < ni)
                        t->n[i] = norms[faceNorms[idx]];
                }

                // If no normals, compute face normal
                if (normCount == 0) {
                    Vec3 fn = t->GetFaceNormal();
                    t->n[0] = t->n[1] = t->n[2] = fn;
                }
            }

            if (faceVertCount == 4) {
                // Triangle 2 (quad)
                Triangle* t = &mesh->tris[mesh->triCount++];
                int indices[3] = {0, 2, 3};
                for (int i = 0; i < 3; i++) {
                    int idx = indices[i];
                    if (faceVerts[idx] > 0 && faceVerts[idx] < vi) t->p[i] = verts[faceVerts[idx]];
                    if (uvs && faceUVs[idx] > 0 && faceUVs[idx] < ui) t->uv[i] = uvs[faceUVs[idx]];
                    if (norms && faceNorms[idx] > 0 && faceNorms[idx] < ni)
                        t->n[i] = norms[faceNorms[idx]];
                }

                if (normCount == 0) {
                    Vec3 fn = t->GetFaceNormal();
                    t->n[0] = t->n[1] = t->n[2] = fn;
                }
            }
        }

        // Skip to next line
        while (ptr < end && *ptr != '\n') ptr++;
        if (ptr < end) ptr++;
    }

    // Cleanup temp arrays
    delete[] verts;
    if (uvs) delete[] uvs;
    if (norms) delete[] norms;

    printf("OBJ Loaded: %d triangles\n", mesh->triCount);
    return mesh;
}

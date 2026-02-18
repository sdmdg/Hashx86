/**
 * @file        Renderer3D.cpp
 * @brief       User-space 3D Rendering Engine
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
}

Renderer3D::~Renderer3D() {
    delete[] this->zBuffer;
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
        clipped[i].pos.z = invZ;  // Store 1/z for z-buffer
    }

    FillTriangle(buffer, clipped[0], clipped[1], clipped[2]);

    if (clipCount == 4) {
        FillTriangle(buffer, clipped[0], clipped[2], clipped[3]);
    }
}

// ============================================================================
// TRIANGLE RASTERIZER (Scanline with Z-buffer, texturing, and lighting)
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

    for (int y = y1; y < y3; y++) {
        if (y < 0 || y >= height) continue;

        // Interpolate along long edge (v1 -> v3)
        float t13 = ((float)y - v1.pos.y) * invDy13;

        float xA = v1.pos.x + (v3.pos.x - v1.pos.x) * t13;
        float zA = v1.pos.z + (v3.pos.z - v1.pos.z) * t13;
        float uA = v1.uv.x + (v3.uv.x - v1.uv.x) * t13;
        float vA = v1.uv.y + (v3.uv.y - v1.uv.y) * t13;
        float lA = v1.light + (v3.light - v1.light) * t13;

        // Interpolate along short edge
        float xB, zB, uB, vB, lB;
        if (y < y2) {
            float dy12 = v2.pos.y - v1.pos.y;
            if (dy12 == 0.0f) continue;
            float t12 = ((float)y - v1.pos.y) / dy12;
            xB = v1.pos.x + (v2.pos.x - v1.pos.x) * t12;
            zB = v1.pos.z + (v2.pos.z - v1.pos.z) * t12;
            uB = v1.uv.x + (v2.uv.x - v1.uv.x) * t12;
            vB = v1.uv.y + (v2.uv.y - v1.uv.y) * t12;
            lB = v1.light + (v2.light - v1.light) * t12;
        } else {
            float dy23 = v3.pos.y - v2.pos.y;
            if (dy23 == 0.0f) continue;
            float t23 = ((float)y - v2.pos.y) / dy23;
            xB = v2.pos.x + (v3.pos.x - v2.pos.x) * t23;
            zB = v2.pos.z + (v3.pos.z - v2.pos.z) * t23;
            uB = v2.uv.x + (v3.uv.x - v2.uv.x) * t23;
            vB = v2.uv.y + (v3.uv.y - v2.uv.y) * t23;
            lB = v2.light + (v3.light - v2.light) * t23;
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
            tmp = uA;
            uA = uB;
            uB = tmp;
            tmp = vA;
            vA = vB;
            vB = tmp;
            tmp = lA;
            lA = lB;
            lB = tmp;
        }

        int xStart = (int)ceilf(xA);
        int xEnd = (int)ceilf(xB);
        if (xStart < 0) xStart = 0;
        if (xEnd > width) xEnd = width;

        float dxAB = xB - xA;
        if (dxAB == 0.0f) continue;
        float invDxAB = 1.0f / dxAB;

        int yOffset = y * width;

        for (int x = xStart; x < xEnd; x++) {
            float t = ((float)x - xA) * invDxAB;

            // Interpolate Z (1/z space)
            float z = zA + (zB - zA) * t;

            int idx = yOffset + x;

            // Z-buffer test (higher 1/z = closer)
            if (z <= zBuffer[idx]) continue;
            zBuffer[idx] = z;

            // Interpolate UV and lighting
            float u = uA + (uB - uA) * t;
            float v = vA + (vB - vA) * t;
            float light = lA + (lB - lA) * t;

            uint32_t color;
            if (hasTexture) {
                // Perspective-correct UV
                float invZ = 1.0f / z;

                // Sample texture
                int texX = (int)(u * texW) & (texW - 1);
                int texY = (int)(v * texH) & (texH - 1);
                if (texX < 0) texX += texW;
                if (texY < 0) texY += texH;

                color = texData[texY * texW + texX];
            } else {
                color = 0xFFCCCCCC;  // Default grey
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

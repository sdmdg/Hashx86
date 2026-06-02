#ifndef RENDERER3D_H
#define RENDERER3D_H

#include <Bitmap.h>
#include <Math3D.h>

struct Light {
    Vec3 direction;
    float intensity;
    Light() : direction(Vec3(0, -1, 0)), intensity(1.0f) {}
    Light(Vec3 dir, float inten) : direction(dir), intensity(inten) {
        direction.Normalize();
    }
};

// Intermediate structure for clipping
struct Vertex {
    Vec3 pos;       // 3D Position (camera space before projection, screen space after)
    Vec2 uv;        // Texture Coord
    Vec3 normal;    // Normal
    Vec3 worldPos;  // World-space position (for shadow lookup)
    float light;    // Pre-calculated light intensity
};

// Shadow map configuration
#define SHADOW_MAP_SIZE 512
#define SHADOW_BIAS 0.5f

class Renderer3D {
private:
    int width, height;
    float* zBuffer;
    float halfWidth, halfHeight;

    // Settings
    bool enableBackfaceCulling;
    bool enableTexturing;

    // Current State
    Bitmap* currentTexture;
    Bitmap* skybox;

    uint32_t texWidthMask, texHeightMask;
    int texWidthShift;

    // Lighting Settings
    float ambientStrength;
    float specularStrength;
    float shininess;
    bool enableZRead;
    bool enableZWrite;
    bool enableLighting;

    // Shadow Map
    float* shadowMap;
    bool shadowsEnabled;
    Vec3 shadowLightDir;    // Normalized light direction for shadow casting
    float shadowOrthoSize;  // Half-size of orthographic shadow volume
    float shadowNear, shadowFar;
    // Shadow light-space transform cache
    float slCosY, slSinY, slCosP, slSinP;
    float slCenterX, slCenterY, slCenterZ;

    // Skybox camera state
    float skyYaw, skyPitch;

    // Helpers
    float ParseFloat(char*& ptr);
    int ParseInt(char*& ptr);
    void SkipWhitespace(char*& ptr);

    float CalculateLighting(const Vec3& normal, const Vec3& viewDir, Light* lights, int lightCount);

    void ClipTriangle(Vertex v1, Vertex v2, Vertex v3, uint32_t* buffer);

    // Shadow map internals
    void RasterizeShadowTriangle(Vec3 p0, Vec3 p1, Vec3 p2);
    float SampleShadow(const Vec3& worldPos);
    Vec3 WorldToShadowUV(const Vec3& worldPos);

public:
    Renderer3D(int w, int h);
    ~Renderer3D();

    void Clear(uint32_t* buffer, uint32_t color);
    void ClearSky(uint32_t* buffer, float camYaw, float camPitch);

    void DrawMesh(uint32_t* buffer, Mesh* mesh, float camX, float camY, float camZ, float camYaw,
                  float camPitch, Light* lights, int lightCount);

    void FillTriangle(uint32_t* buffer, Vertex v1, Vertex v2, Vertex v3);

    void BindTexture(Bitmap* texture);
    void SetSkybox(Bitmap* skyTexture);
    void SetMaterial(float ambient, float specular, float shininess);
    Mesh* LoadOBJ(uint8_t* data, uint32_t dataSize);

    // Shadow Map API
    void SetupShadows(const Vec3& lightDir, float orthoSize, float nearPlane, float farPlane);
    void BeginShadowPass(float centerX, float centerY, float centerZ);
    void RenderMeshToShadowMap(Mesh* mesh);
    void EndShadowPass();
    void EnableShadows(bool enable) {
        shadowsEnabled = enable;
    }
};

#endif

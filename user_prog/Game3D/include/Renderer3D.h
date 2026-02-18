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
    Vec3 pos;     // 3D World Position
    Vec2 uv;      // Texture Coord
    Vec3 normal;  // Normal
    float light;  // Pre-calculated light intensity
};

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

    // Helpers
    float ParseFloat(char*& ptr);
    int ParseInt(char*& ptr);
    void SkipWhitespace(char*& ptr);

    float CalculateLighting(const Vec3& normal, const Vec3& viewDir, Light* lights, int lightCount);

    void ClipTriangle(Vertex v1, Vertex v2, Vertex v3, uint32_t* buffer);

public:
    Renderer3D(int w, int h);
    ~Renderer3D();

    void Clear(uint32_t* buffer, uint32_t color);

    void DrawMesh(uint32_t* buffer, Mesh* mesh, float camX, float camY, float camZ, float camYaw,
                  float camPitch, Light* lights, int lightCount);

    void FillTriangle(uint32_t* buffer, Vertex v1, Vertex v2, Vertex v3);

    void BindTexture(Bitmap* texture);
    void SetSkybox(Bitmap* skyTexture);
    void SetMaterial(float ambient, float specular, float shininess);
    Mesh* LoadOBJ(uint8_t* data, uint32_t dataSize);
};

#endif

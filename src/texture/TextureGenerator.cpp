#include "texture/TextureGenerator.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <filesystem>
#include <algorithm>

namespace {

// Smooth deterministic 2D hash and noise primitives
float hash2D(float x, float y)
{
    float n = std::sin(x * 127.1f + y * 311.7f) * 43758.5453123f;
    return n - std::floor(n);
}

float smoothNoise2D(float x, float y)
{
    float iX = std::floor(x);
    float iY = std::floor(y);
    float fX = x - iX;
    float fY = y - iY;

    // Cubic hermite interpolation
    float uX = fX * fX * (3.0f - 2.0f * fX);
    float uY = fY * fY * (3.0f - 2.0f * fY);

    float a = hash2D(iX,        iY);
    float b = hash2D(iX + 1.0f, iY);
    float c = hash2D(iX,        iY + 1.0f);
    float d = hash2D(iX + 1.0f, iY + 1.0f);

    return a + (b - a) * uX + (c - a) * uY + (a - b - c + d) * uX * uY;
}

float fbm2D(float x, float y, int octaves = 4)
{
    float val = 0.0f;
    float amp = 0.5f;
    float freq = 1.0f;
    for (int i = 0; i < octaves; ++i)
    {
        val += smoothNoise2D(x * freq, y * freq) * amp;
        freq *= 2.02f;
        amp *= 0.5f;
    }
    return val;
}

unsigned char clampByte(float val)
{
    int i = static_cast<int>(std::round(val));
    if (i < 0) return 0;
    if (i > 255) return 255;
    return static_cast<unsigned char>(i);
}

} // anonymous namespace

bool TextureGenerator::saveBMP(const std::string& filePath, int width, int height, const unsigned char* rgbData)
{
    std::ofstream out(filePath, std::ios::binary);
    if (!out.is_open())
    {
        return false;
    }

    int rowStride = width * 3;
    int padding = (4 - (rowStride % 4)) % 4;
    int paddedRowSize = rowStride + padding;
    int pixelDataSize = paddedRowSize * height;
    int fileSize = 54 + pixelDataSize;

    // BMP Header (14 bytes)
    unsigned char fileHeader[14] = {
        'B', 'M',
        static_cast<unsigned char>(fileSize & 0xFF),
        static_cast<unsigned char>((fileSize >> 8) & 0xFF),
        static_cast<unsigned char>((fileSize >> 16) & 0xFF),
        static_cast<unsigned char>((fileSize >> 24) & 0xFF),
        0, 0, 0, 0,
        54, 0, 0, 0
    };

    // DIB Header (40 bytes - BITMAPINFOHEADER)
    unsigned char dibHeader[40] = {
        40, 0, 0, 0,
        static_cast<unsigned char>(width & 0xFF),
        static_cast<unsigned char>((width >> 8) & 0xFF),
        static_cast<unsigned char>((width >> 16) & 0xFF),
        static_cast<unsigned char>((width >> 24) & 0xFF),
        static_cast<unsigned char>(height & 0xFF),
        static_cast<unsigned char>((height >> 8) & 0xFF),
        static_cast<unsigned char>((height >> 16) & 0xFF),
        static_cast<unsigned char>((height >> 24) & 0xFF),
        1, 0,
        24, 0,
        0, 0, 0, 0,
        static_cast<unsigned char>(pixelDataSize & 0xFF),
        static_cast<unsigned char>((pixelDataSize >> 8) & 0xFF),
        static_cast<unsigned char>((pixelDataSize >> 16) & 0xFF),
        static_cast<unsigned char>((pixelDataSize >> 24) & 0xFF),
        0x13, 0x0B, 0, 0, // ~2835 pixels/meter
        0x13, 0x0B, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    out.write(reinterpret_cast<const char*>(fileHeader), 14);
    out.write(reinterpret_cast<const char*>(dibHeader), 40);

    std::vector<unsigned char> padBytes(padding, 0);

    // BMP stores pixels bottom-to-top, in BGR order
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = (y * width + x) * 3;
            unsigned char r = rgbData[idx + 0];
            unsigned char g = rgbData[idx + 1];
            unsigned char b = rgbData[idx + 2];

            unsigned char bgr[3] = { b, g, r };
            out.write(reinterpret_cast<const char*>(bgr), 3);
        }
        if (padding > 0)
        {
            out.write(reinterpret_cast<const char*>(padBytes.data()), padding);
        }
    }

    return true;
}

std::vector<unsigned char> TextureGenerator::generateGrassEarth(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            // Multi-frequency organic noise
            float baseNoise = fbm2D(u * 8.0f, v * 8.0f, 4);
            float fineBlades = smoothNoise2D(u * 64.0f, v * 64.0f);
            float soilPatch = smoothNoise2D(u * 4.0f, v * 4.0f);

            // Lush green vs golden-dry blade variation
            float r = 70.0f  + 50.0f * baseNoise + 30.0f * fineBlades;
            float g = 120.0f + 65.0f * baseNoise + 40.0f * fineBlades;
            float b = 35.0f  + 25.0f * baseNoise + 15.0f * fineBlades;

            // Soil blend in lower crevices
            if (soilPatch < 0.32f)
            {
                float blend = (0.32f - soilPatch) / 0.32f;
                r = r * (1.0f - blend) + 95.0f * blend;
                g = g * (1.0f - blend) + 65.0f * blend;
                b = b * (1.0f - blend) + 40.0f * blend;
            }

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

std::vector<unsigned char> TextureGenerator::generateRockGranite(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            // Natural granite rock grain with veins
            float strata = fbm2D(u * 12.0f + v * 2.0f, v * 12.0f, 5);
            float fineGrain = smoothNoise2D(u * 96.0f, v * 96.0f);
            float quartzVein = std::abs(smoothNoise2D(u * 6.0f, v * 6.0f) - 0.5f) * 2.0f;
            quartzVein = std::pow(1.0f - quartzVein, 6.0f);

            float baseVal = 100.0f + 70.0f * strata + 35.0f * fineGrain + 65.0f * quartzVein;

            // Subtle cool granite tint
            float r = baseVal * 0.98f;
            float g = baseVal * 1.00f;
            float b = baseVal * 1.05f;

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

std::vector<unsigned char> TextureGenerator::generateWoodTimber(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            // Realistic concentric growth rings with longitudinal grain wave
            float grainWobble = fbm2D(u * 4.0f, v * 1.5f, 3) * 0.45f;
            float radius = std::sqrt(std::pow(u * 3.5f - 1.75f + grainWobble, 2.0f) + std::pow((v * 1.2f - 0.6f) * 0.25f, 2.0f));

            float ringFreq = 34.0f;
            float ringPattern = 0.5f + 0.5f * std::sin(radius * ringFreq);
            float fineFibers = smoothNoise2D(u * 160.0f, v * 8.0f) * 0.35f;

            float intensity = ringPattern * 0.40f + fineFibers + 0.45f;

            // Rich warm golden honey-oak tone
            float r = 165.0f * intensity + 25.0f;
            float g = 110.0f * intensity + 15.0f;
            float b = 60.0f  * intensity + 10.0f;

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

std::vector<unsigned char> TextureGenerator::generateCobblestone(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    const int cols = 8;
    const int rows = 12;
    float cellW = 1.0f / static_cast<float>(cols);
    float cellH = 1.0f / static_cast<float>(rows);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            // Offset every second row like real brick / cobblestone paving
            int rowIdx = static_cast<int>(v / cellH);
            float rowOffset = (rowIdx % 2 == 1) ? (cellW * 0.5f) : 0.0f;

            float cellU = std::fmod(u + rowOffset, cellW) / cellW;
            float cellV = std::fmod(v, cellH) / cellH;

            // Distance to stone edges
            float dEdgeX = std::min(cellU, 1.0f - cellU);
            float dEdgeY = std::min(cellV, 1.0f - cellV);
            float dMortar = std::min(dEdgeX, dEdgeY);

            float noise = smoothNoise2D(u * 32.0f, v * 32.0f);
            float stoneHash = hash2D(static_cast<float>(static_cast<int>((u + rowOffset) / cellW)), static_cast<float>(rowIdx));

            float r, g, b;
            if (dMortar < 0.09f)
            {
                // Dark recessed mortar grout channel
                float mortarDepth = dMortar / 0.09f;
                r = 45.0f  + 20.0f * mortarDepth + 15.0f * noise;
                g = 42.0f  + 18.0f * mortarDepth + 15.0f * noise;
                b = 40.0f  + 18.0f * mortarDepth + 15.0f * noise;
            }
            else
            {
                // Rounded cobblestone surface with bevel curve
                float bevel = std::pow(std::min(1.0f, (dMortar - 0.09f) * 4.5f), 0.45f);
                float stoneTone = (0.75f + 0.35f * stoneHash) * bevel;

                r = (120.0f + 40.0f * noise) * stoneTone;
                g = (112.0f + 35.0f * noise) * stoneTone;
                b = (108.0f + 35.0f * noise) * stoneTone;
            }

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

std::vector<unsigned char> TextureGenerator::generateRoofShingles(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    const int courses = 16;
    float courseH = 1.0f / static_cast<float>(courses);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            int courseIdx = static_cast<int>(v / courseH);
            float inCourseV = std::fmod(v, courseH) / courseH;

            // Slit stagger between adjacent shingles
            float slitPeriod = 0.08f;
            float inCourseU = std::fmod(u + courseIdx * 0.04f, slitPeriod) / slitPeriod;
            float slitEdge = std::min(inCourseU, 1.0f - inCourseU);

            float noise = smoothNoise2D(u * 28.0f, v * 14.0f);

            // Shingle slope gradient (shadow under overlap at top, highlight at bottom lip)
            float courseGradient = inCourseV * inCourseV;
            float shade = 0.55f + 0.45f * courseGradient;

            if (slitEdge < 0.04f)
            {
                shade *= 0.65f; // Vertical seam shadow
            }

            // Dark weathered slate / charcoal wood shake tone
            float r = (90.0f  + 35.0f * noise) * shade;
            float g = (85.0f  + 30.0f * noise) * shade;
            float b = (95.0f  + 38.0f * noise) * shade;

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

std::vector<unsigned char> TextureGenerator::generateMetalIron(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            // Anisotropic fine brushed lines along horizontal axis
            float brush = smoothNoise2D(u * 12.0f, v * 180.0f);
            float subtlePatina = fbm2D(u * 6.0f, v * 6.0f, 3) * 0.25f;

            float val = 110.0f + 40.0f * brush + 25.0f * subtlePatina;

            // Gunmetal cold grey
            float r = val * 0.94f;
            float g = val * 0.96f;
            float b = val * 1.04f;

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

std::vector<unsigned char> TextureGenerator::generateTreeBark(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            // Deep vertical furrowed bark ridges with turbulence
            float furrowWave = fbm2D(u * 16.0f, v * 3.5f, 5);
            float verticalRidge = std::abs(std::sin((u * 18.0f + furrowWave * 1.2f) * 3.14159f));

            float depth = std::pow(verticalRidge, 0.65f);
            float fineCracks = smoothNoise2D(u * 80.0f, v * 24.0f) * 0.25f;

            float shade = depth * 0.75f + fineCracks + 0.25f;

            // Weathered pine bark palette
            float r = (95.0f  + 20.0f * furrowWave) * shade;
            float g = (65.0f  + 15.0f * furrowWave) * shade;
            float b = (45.0f  + 12.0f * furrowWave) * shade;

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

std::vector<unsigned char> TextureGenerator::generatePineLeaf(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 3);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float u = static_cast<float>(x) / static_cast<float>(width);
            float v = static_cast<float>(y) / static_cast<float>(height);

            // Layered pine needle striations with micro-veins
            float fiberNoise = fbm2D(u * 14.0f, v * 14.0f, 4);
            // Fine diagonal/radial needle grain
            float needleWave1 = std::abs(std::sin((u * 48.0f + v * 32.0f + fiberNoise * 1.8f) * 3.14159f));
            float needleWave2 = std::abs(std::sin((u * -32.0f + v * 48.0f + fiberNoise * 1.2f) * 3.14159f));
            float needleGrain = std::min(needleWave1, needleWave2);

            // High-frequency micro-fibers along needle surface
            float microFibers = smoothNoise2D(u * 128.0f, v * 96.0f) * 0.28f;
            // Cross-cellular leaf cells
            float leafCells   = smoothNoise2D(u * 24.0f, v * 24.0f) * 0.20f;

            float intensity = 0.68f + 0.32f * std::pow(needleGrain, 0.60f) + microFibers + leafCells;

            // Balanced leaf fiber albedo (multiplies into surface color cleanly)
            float r = 160.0f * intensity;
            float g = 205.0f * intensity;
            float b = 150.0f * intensity;

            int idx = (y * width + x) * 3;
            pixels[idx + 0] = clampByte(r);
            pixels[idx + 1] = clampByte(g);
            pixels[idx + 2] = clampByte(b);
        }
    }
    return pixels;
}

void TextureGenerator::ensureTexturesExist(const std::string& textureDir)
{
    std::filesystem::create_directories(textureDir);

    struct TexDef {
        std::string filename;
        std::vector<unsigned char>(*generator)(int, int);
    };

    TexDef defs[] = {
        { "grass_earth.bmp",    &generateGrassEarth },
        { "rock_granite.bmp",   &generateRockGranite },
        { "wood_timber.bmp",    &generateWoodTimber },
        { "cobblestone.bmp",    &generateCobblestone },
        { "roof_shingles.bmp",  &generateRoofShingles },
        { "metal_iron.bmp",     &generateMetalIron },
        { "bark.bmp",           &generateTreeBark },
        { "leaf_pine.bmp",      &generatePineLeaf }
    };

    for (const auto& def : defs)
    {
        std::string fullPath = textureDir + "/" + def.filename;
        if (!std::filesystem::exists(fullPath))
        {
            std::cout << "[TextureGenerator] Generating realistic texture map: " << fullPath << "..." << std::endl;
            auto data = def.generator(512, 512);
            saveBMP(fullPath, 512, 512, data.data());
        }
    }
}

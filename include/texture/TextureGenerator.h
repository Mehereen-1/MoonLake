#ifndef TEXTURE_GENERATOR_H
#define TEXTURE_GENERATOR_H

#include <string>
#include <vector>

class TextureGenerator
{
public:
    // Ensures all essential realistic texture map files exist in the specified directory.
    // If files are missing, generates them procedurally and saves them as BMP images.
    static void ensureTexturesExist(const std::string& textureDir = "textures");

    // Procedural generation functions (each returns 512x512 RGB pixel data)
    static std::vector<unsigned char> generateGrassEarth(int width = 512, int height = 512);
    static std::vector<unsigned char> generateRockGranite(int width = 512, int height = 512);
    static std::vector<unsigned char> generateWoodTimber(int width = 512, int height = 512);
    static std::vector<unsigned char> generateCobblestone(int width = 512, int height = 512);
    static std::vector<unsigned char> generateRoofShingles(int width = 512, int height = 512);
    static std::vector<unsigned char> generateMetalIron(int width = 512, int height = 512);
    static std::vector<unsigned char> generateTreeBark(int width = 512, int height = 512);
    static std::vector<unsigned char> generatePineLeaf(int width = 512, int height = 512);

    // Save RGB byte buffer as a standard uncompressed 24-bit BMP image
    static bool saveBMP(const std::string& filePath, int width, int height, const unsigned char* rgbData);
};

#endif // TEXTURE_GENERATOR_H

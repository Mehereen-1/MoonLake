#include "texture/TextureManager.h"
#include "texture/TextureGenerator.h"
#include <iostream>

TextureManager::TextureManager()
    : m_initialized(false), m_enabled(true)
{
}

TextureManager& TextureManager::getInstance()
{
    static TextureManager instance;
    return instance;
}

bool TextureManager::init(const std::string& textureDir)
{
    if (m_initialized)
    {
        return true;
    }

    std::cout << "[TextureManager] Initializing realistic texture library from: " << textureDir << std::endl;

    // 1. Ensure texture asset files exist (generates them procedurally if needed)
    TextureGenerator::ensureTexturesExist(textureDir);

    // 2. Load textures into GPU VRAM
    bool ok1 = m_texGrass.loadFromFile(textureDir + "/grass_earth.bmp");
    bool ok2 = m_texRock.loadFromFile(textureDir + "/rock_granite.bmp");
    bool ok3 = m_texWood.loadFromFile(textureDir + "/wood_timber.bmp");
    bool ok4 = m_texCobblestone.loadFromFile(textureDir + "/cobblestone.bmp");
    bool ok5 = m_texRoof.loadFromFile(textureDir + "/roof_shingles.bmp");
    bool ok6 = m_texMetal.loadFromFile(textureDir + "/metal_iron.bmp");
    bool ok7 = m_texBark.loadFromFile(textureDir + "/bark.bmp");
    bool ok8 = m_texLeaf.loadFromFile(textureDir + "/leaf_pine.bmp");

    m_initialized = (ok1 && ok2 && ok3 && ok4 && ok5 && ok6 && ok7 && ok8);

    if (m_initialized)
    {
        std::cout << "[TextureManager] All 8 realistic material textures initialized successfully!" << std::endl;
    }
    else
    {
        std::cerr << "[TextureManager] Warning: Some textures failed to initialize." << std::endl;
    }

    return m_initialized;
}

void TextureManager::bindTextures(const Shader& shader) const
{
    shader.use();

    m_texGrass.bind(0);
    m_texRock.bind(1);
    m_texWood.bind(2);
    m_texCobblestone.bind(3);
    m_texRoof.bind(4);
    m_texMetal.bind(5);
    m_texBark.bind(6);
    m_texLeaf.bind(7);

    shader.setInt("texGrass", 0);
    shader.setInt("texRock", 1);
    shader.setInt("texWood", 2);
    shader.setInt("texCobblestone", 3);
    shader.setInt("texRoof", 4);
    shader.setInt("texMetal", 5);
    shader.setInt("texBark", 6);
    shader.setInt("texLeaf", 7);

    shader.setBool("texturesEnabled", m_enabled);
}

void TextureManager::setTextureMode(const Shader& shader, TextureMode mode, float tiling) const
{
    shader.setInt("textureMode", static_cast<int>(mode));
    shader.setFloat("textureTiling", tiling);
    shader.setBool("texturesEnabled", m_enabled);
}

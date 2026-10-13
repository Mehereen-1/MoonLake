#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "texture/Texture.h"
#include <iostream>
#include <filesystem>

Texture::Texture()
    : m_id(0), m_width(0), m_height(0), m_channels(0)
{
}

Texture::~Texture()
{
    cleanup();
}

void Texture::cleanup()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

bool Texture::loadFromFile(const std::string& filePath, bool generateMipmaps)
{
    cleanup();

    // Candidate lookup paths for flexible runtime directory resolution
    std::string candidatePaths[] = {
        filePath,
        "../" + filePath,
        "../../" + filePath,
        std::string(std::filesystem::current_path().string()) + "/" + filePath
    };

    unsigned char* data = nullptr;
    std::string resolvedPath;

    stbi_set_flip_vertically_on_load(1);

    for (const auto& path : candidatePaths)
    {
        data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 0);
        if (data)
        {
            resolvedPath = path;
            break;
        }
    }

    if (!data)
    {
        std::cerr << "[Texture] Failed to load image from file: " << filePath
                  << " (Reason: " << stbi_failure_reason() << ")" << std::endl;
        return false;
    }

    bool success = createFromData(m_width, m_height, m_channels, data, generateMipmaps);
    stbi_image_free(data);

    if (success)
    {
        std::cout << "[Texture] Loaded successfully: " << resolvedPath
                  << " (" << m_width << "x" << m_height << ", " << m_channels << " channels)" << std::endl;
    }
    return success;
}

bool Texture::createFromData(int width, int height, int channels, const unsigned char* data, bool generateMipmaps)
{
    if (!data || width <= 0 || height <= 0)
    {
        return false;
    }

    cleanup();

    m_width = width;
    m_height = height;
    m_channels = channels;

    GLenum internalFormat = GL_RGB;
    GLenum dataFormat = GL_RGB;

    if (channels == 1)
    {
        internalFormat = GL_RED;
        dataFormat = GL_RED;
    }
    else if (channels == 3)
    {
        internalFormat = GL_RGB;
        dataFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        internalFormat = GL_RGBA;
        dataFormat = GL_RGBA;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    // Texture wrapping: GL_REPEAT for tileable materials
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Texture filtering: trilinear filtering with mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixel data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

    if (generateMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Texture::bind(unsigned int unit) const
{
    if (m_id != 0)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

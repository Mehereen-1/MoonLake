#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

static const char* DEFAULT_VERTEX_SOURCE = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * aNormal);
    vertexColor = aColor;
    gl_Position = projection * view * worldPos;
}
)";

static const char* DEFAULT_FRAGMENT_SOURCE = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 vertexColor;
out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 moonLightDir;
uniform vec3 moonLightColor;
uniform vec3 ambientColor;
uniform bool isEmissive;
uniform vec3 emissionColor;
uniform float shininess;
uniform bool isMoon;
uniform bool isMoonGlow;
uniform bool isSun;
uniform bool isSunGlow;
uniform float sunsetFactor;
uniform float celestialAlpha;
uniform float time;
uniform bool disableLighting;

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};
uniform SpotLight trainSpotLight;
uniform bool useTrainSpotLight;

float hash(vec3 p) {
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(mix(hash(i + vec3(0,0,0)), hash(i + vec3(1,0,0)), f.x),
            mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash(i + vec3(0,0,1)), hash(i + vec3(1,0,1)), f.x),
            mix(hash(i + vec3(0,1,1)), hash(i + vec3(1,1,1)), f.x), f.y),
        f.z
    );
}

float fbm(vec3 p) {
    float f = 0.0;
    f += 0.5000 * noise(p * 2.8);
    f += 0.2500 * noise(p * 5.6);
    f += 0.1250 * noise(p * 11.2);
    f += 0.0625 * noise(p * 22.4);
    return f;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    if (isMoonGlow) {
        vec2 coord = (length(vertexColor.xy) > 0.001) ? vertexColor.xy : Normal.xy;
        float dist = length(coord);
        if (dist > 1.0) discard;
        float innerGlow = exp(-dist * 4.0);
        float outerHaze = exp(-dist * 1.8) * 0.45;
        float rimFade = smoothstep(1.0, 0.35, dist);
        float effAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        float alpha = (innerGlow + outerHaze) * rimFade * 0.85 * effAlpha;
        vec3 glowColor = vec3(0.65, 0.78, 0.95);
        FragColor = vec4(glowColor * alpha, alpha);
        return;
    }

    if (isMoon) {
        float maria = fbm(norm * 3.2);
        float craters = fbm(norm * 12.0);
        vec3 highlandColor = vec3(0.96, 0.97, 0.94);
        vec3 mariaColor = vec3(0.62, 0.66, 0.70);
        vec3 craterDark = vec3(0.52, 0.55, 0.58);
        vec3 moonColor = mix(mariaColor, highlandColor, smoothstep(0.38, 0.62, maria));
        moonColor = mix(moonColor, craterDark, smoothstep(0.70, 0.85, craters) * 0.35);

        float NdotV = max(dot(norm, viewDir), 0.0);
        float limbDarkening = pow(NdotV, 0.35);
        vec3 moonSunDir = normalize(vec3(0.35, 0.45, 0.82));
        float hemisphere3D = max(dot(norm, moonSunDir), 0.0) * 0.28 + 0.72;

        vec3 finalMoon = moonColor * limbDarkening * hemisphere3D * 1.15;
        float effAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        FragColor = vec4(finalMoon, effAlpha);
        return;
    }

    if (isSunGlow) {
        vec2 coord = (length(vertexColor.xy) > 0.001) ? vertexColor.xy : Normal.xy;
        float dist = length(coord);
        if (dist > 1.0) discard;
        float innerCorona = exp(-dist * 4.8) * 1.30;
        float middleHalo   = exp(-dist * 2.2) * 0.65;
        float outerHaze   = exp(-dist * 1.1) * 0.30;
        float rimFade     = smoothstep(1.0, 0.30, dist);
        float angle = atan(coord.y, coord.x);
        float streamers = 0.92 + 0.08 * sin(angle * 10.0 + time * 0.35) * cos(angle * 6.0 - time * 0.20);
        float effAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        float alpha = (innerCorona + middleHalo + outerHaze) * rimFade * streamers * effAlpha;

        vec3 dayCoreGlow   = vec3(1.35, 1.25, 0.98);
        vec3 dayMidGlow    = vec3(1.10, 0.88, 0.48);
        vec3 dayOuterGlow  = vec3(0.92, 0.75, 0.40);
        vec3 sunsetCoreGlow  = vec3(1.35, 0.90, 0.45);
        vec3 sunsetMidGlow   = vec3(1.15, 0.52, 0.18);
        vec3 sunsetOuterGlow = vec3(0.90, 0.32, 0.08);

        vec3 coreGlow  = mix(dayCoreGlow,  sunsetCoreGlow,  sunsetFactor);
        vec3 midGlow   = mix(dayMidGlow,   sunsetMidGlow,   sunsetFactor);
        vec3 outerGlow = mix(dayOuterGlow, sunsetOuterGlow, sunsetFactor);
        vec3 glowCol = mix(coreGlow, midGlow, smoothstep(0.0, 0.45, dist));
        glowCol = mix(glowCol, outerGlow, smoothstep(0.45, 0.90, dist));

        FragColor = vec4(glowCol * alpha, alpha);
        return;
    }

    if (isSun) {
        vec3 pNorm = norm * 4.5 + vec3(time * 0.03, -time * 0.02, time * 0.025);
        float granulation = fbm(pNorm);
        float finePlasma = noise(norm * 14.0 + vec3(-time * 0.06, time * 0.05, 0.0));
        float plasmaTurbulence = granulation * 0.70 + finePlasma * 0.30;

        float NdotV = max(dot(norm, viewDir), 0.0);
        float limbDarkening = pow(NdotV, 0.42);

        vec3 dayCore  = vec3(1.35, 1.30, 1.12);
        vec3 dayMid   = vec3(1.15, 0.95, 0.58);
        vec3 dayLimb  = vec3(1.00, 0.70, 0.28);
        vec3 sunsetCore = vec3(1.30, 0.95, 0.55);
        vec3 sunsetMid  = vec3(1.18, 0.55, 0.18);
        vec3 sunsetLimb = vec3(0.92, 0.30, 0.08);

        vec3 coreColor = mix(dayCore, sunsetCore, sunsetFactor);
        vec3 midColor  = mix(dayMid,  sunsetMid,  sunsetFactor);
        vec3 limbColor = mix(dayLimb, sunsetLimb, sunsetFactor);

        vec3 sunColor = mix(limbColor, midColor, smoothstep(0.0, 0.52, limbDarkening));
        sunColor = mix(sunColor, coreColor, smoothstep(0.52, 1.0, limbDarkening));
        sunColor *= (0.91 + 0.18 * plasmaTurbulence);

        float rimFresnel = pow(1.0 - NdotV, 3.2);
        vec3 rimColor = mix(vec3(1.05, 0.85, 0.40), vec3(1.05, 0.45, 0.12), sunsetFactor);
        sunColor += rimColor * rimFresnel * 0.85;

        float effAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        FragColor = vec4(sunColor, effAlpha);
        return;
    }

    if (isEmissive) {
        FragColor = vec4(vertexColor * emissionColor, 1.0);
        return;
    }

    vec3 lightDir = normalize(moonLightDir);
    vec3 ambient = ambientColor * vertexColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * moonLightColor * vertexColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess > 0.0 ? shininess : 32.0);
    vec3 specular = spec * moonLightColor * 0.35;

    vec3 result = ambient + diffuse + specular;

    if (useTrainSpotLight) {
        vec3 sLightDir = normalize(trainSpotLight.position - FragPos);
        float sDist = length(trainSpotLight.position - FragPos);
        float theta = dot(-sLightDir, normalize(trainSpotLight.direction));
        float epsilon = trainSpotLight.cutOff - trainSpotLight.outerCutOff;
        float spotIntensity = clamp((theta - trainSpotLight.outerCutOff) / (epsilon > 0.0001 ? epsilon : 0.0001), 0.0, 1.0);
        if (spotIntensity > 0.0) {
            float sAtten = 1.0 / (trainSpotLight.constant + trainSpotLight.linear * sDist + trainSpotLight.quadratic * (sDist * sDist));
            float sDiff = max(dot(norm, sLightDir), 0.0);
            vec3 sDiffuse = sDiff * trainSpotLight.color * vertexColor;
            vec3 sHalfway = normalize(sLightDir + viewDir);
            float sSpec = pow(max(dot(norm, sHalfway), 0.0), shininess > 0.0 ? shininess : 32.0);
            vec3 sSpecular = sSpec * trainSpotLight.color * 0.45;
            result += (sDiffuse + sSpecular) * (sAtten * spotIntensity) * trainSpotLight.intensity;
        }
    }

    FragColor = vec4(result, 1.0);
}
)";

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexCode = readFile(vertexPath, DEFAULT_VERTEX_SOURCE);
    std::string fragmentCode = readFile(fragmentPath, DEFAULT_FRAGMENT_SOURCE);

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexCode);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(ID, 1024, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog
                  << "\n--------------------------------------------------" << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader()
{
    if (ID != 0)
    {
        glDeleteProgram(ID);
    }
}

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

std::string Shader::preprocessIncludes(const std::string& source, const std::string& sourceDir, int depth) const
{
    if (depth > 8)
    {
        std::cerr << "[Shader] Warning: Max include recursion depth reached!" << std::endl;
        return source;
    }

    std::istringstream stream(source);
    std::ostringstream output;
    std::string line;

    while (std::getline(stream, line))
    {
        // Trim leading whitespace for #include check
        size_t firstNonSpace = line.find_first_not_of(" \t\r");
        if (firstNonSpace != std::string::npos && line.substr(firstNonSpace, 8) == "#include")
        {
            size_t startQuote = line.find('"', firstNonSpace + 8);
            size_t endQuote = (startQuote != std::string::npos) ? line.find('"', startQuote + 1) : std::string::npos;
            if (startQuote == std::string::npos || endQuote == std::string::npos)
            {
                startQuote = line.find('<', firstNonSpace + 8);
                endQuote = (startQuote != std::string::npos) ? line.find('>', startQuote + 1) : std::string::npos;
            }

            if (startQuote != std::string::npos && endQuote != std::string::npos && endQuote > startQuote + 1)
            {
                std::string includeFile = line.substr(startQuote + 1, endQuote - startQuote - 1);
                std::string includePath = sourceDir.empty() ? includeFile : (sourceDir + "/" + includeFile);

                // Read included file
                std::string includedSource = readFile(includePath, "");
                if (includedSource.empty() && !sourceDir.empty())
                {
                    includedSource = readFile("shaders/" + includeFile, "");
                    if (includedSource.empty())
                    {
                        includedSource = readFile(includeFile, "");
                    }
                }

                if (!includedSource.empty())
                {
                    std::filesystem::path p(includePath);
                    std::string nextDir = p.has_parent_path() ? p.parent_path().string() : sourceDir;
                    output << "// --- BEGIN INCLUDED: " << includeFile << " ---\n";
                    output << preprocessIncludes(includedSource, nextDir, depth + 1) << "\n";
                    output << "// --- END INCLUDED: " << includeFile << " ---\n";
                    continue;
                }
                else
                {
                    std::cerr << "[Shader] Error: Could not resolve #include file: " << includeFile << std::endl;
                }
            }
        }
        output << line << "\n";
    }

    return output.str();
}

std::string Shader::readFile(const std::string& filePath, const std::string& fallback) const
{
    std::string candidatePaths[] = {
        filePath,
        "../" + filePath,
        std::string(std::filesystem::current_path().string()) + "/" + filePath
    };

    for (const auto& path : candidatePaths)
    {
        std::ifstream file(path);
        if (file.is_open())
        {
            std::stringstream stream;
            stream << file.rdbuf();
            std::string content = stream.str();

            // Extract directory of this file
            std::filesystem::path p(filePath);
            std::string sourceDir = p.has_parent_path() ? p.parent_path().string() : "";

            return preprocessIncludes(content, sourceDir, 0);
        }
    }

    if (!fallback.empty())
    {
        std::cout << "[Shader] Using embedded fallback for: " << filePath << std::endl;
        return fallback;
    }
    return "";
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) const
{
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED of type: "
                  << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "\n"
                  << infoLog << "\n--------------------------------------------------" << std::endl;
    }
    return shader;
}

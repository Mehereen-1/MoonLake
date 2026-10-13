#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 vertexColor;
in vec2 LakeUV;
in float ShoreDist;

out vec4 FragColor;

// ---------------------------------------------------------------------------
// Real Lake Water Physical Lighting & Optical Uniforms
// ---------------------------------------------------------------------------
uniform vec3 viewPos;
uniform vec3 moonLightDir;
uniform vec3 moonLightColor;
uniform vec3 ambientColor;
uniform vec3 skyHorizonColor;
uniform vec3 waterBaseColor;
uniform float time;

// Dynamic Campfire Water Reflection Uniforms
uniform vec3 campfirePos;
uniform vec3 campfireColor;
uniform float campfireIntensity;
uniform bool hasCampfire;

// Dynamic Autumn Dock Lantern Water Reflection Uniforms
uniform vec3 lanternPos;
uniform vec3 lanternColor;
uniform float lanternIntensity;
uniform bool hasLantern;

void main()
{
    vec3 lightDir = normalize(moonLightDir);
    vec3 viewDir = normalize(viewPos - FragPos);

    // -----------------------------------------------------------------------
    // 1. Procedural Micro-Capillary Ripples (Subtle, smooth liquid motion)
    // -----------------------------------------------------------------------
    vec2 uv1 = FragPos.xz * 1.6 + vec2(time * 0.08, time * 0.05);
    vec2 uv2 = FragPos.xz * 3.2 - vec2(time * 0.06, -time * 0.07);

    float r1 = sin(uv1.x * 2.2 + cos(uv1.y * 1.8));
    float r2 = cos(uv2.y * 2.4 + sin(uv2.x * 2.0));

    vec3 microOffset = vec3(
        r1 * 0.022 + r2 * 0.014,
        1.0,
        cos(uv1.y * 2.2) * 0.022 + sin(uv2.x * 2.4) * 0.014
    );

    // Blend: 88% smooth geometric wave normal + 12% delicate micro-ripples
    // (Prevents normal spiking that previously caused glaring white blobs!)
    vec3 norm = normalize(Normal * 0.88 + normalize(microOffset) * 0.12);

    // -----------------------------------------------------------------------
    // 2. Realistic Mountain Lake Water Depth & Shoreline Gradient
    // -----------------------------------------------------------------------
    // Deep center: Rich alpine navy / sapphire blue
    // Shallow banks: Transparent emerald/turquoise clear water
    float depthFactor = clamp(1.0 - smoothstep(0.12, 0.88, ShoreDist), 0.0, 1.0);

    vec3 deepWaterCol = (waterBaseColor != vec3(0.0)) ? waterBaseColor : vec3(0.012, 0.038, 0.078);
    vec3 shallowWaterCol = vec3(0.045, 0.160, 0.185); // Clear emerald shoreline
    vec3 waterBodyColor = mix(shallowWaterCol, deepWaterCol, depthFactor);

    // Soft ambient lighting through water column
    vec3 ambient = ambientColor * waterBodyColor * 1.8;

    // Diffuse subsurface scattering
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * moonLightColor * waterBodyColor * 0.45;

    // -----------------------------------------------------------------------
    // 3. Physical Schlick Fresnel Reflection (Air-to-Water n = 1.333, R0 = 0.0203)
    // -----------------------------------------------------------------------
    // Grazing angle: mirror reflection of sky & horizon
    // Steep/downward angle: clear water body showing depth
    float NdotV = max(dot(norm, viewDir), 0.0);
    float fresnel = 0.0203 + 0.9797 * pow(1.0 - NdotV, 5.0);

    // Soft sky reflection across the water surface
    vec3 reflectedSky = mix(skyHorizonColor * 0.80, ambientColor * 1.6, 0.35);

    // Base water composition
    vec3 waterSurface = mix(waterBodyColor + ambient + diffuse, reflectedSky, fresnel * 0.75);

    // -----------------------------------------------------------------------
    // 4. Directional Sun / Moon Glitter Path (Realistic "Sun Road")
    // -----------------------------------------------------------------------
    // In real life, specular highlight appears ONLY in a direct alignment
    // path towards the sun/moon, NOT across every wave on the entire lake!
    vec2 viewXZ = normalize(viewDir.xz);
    vec2 lightXZ = normalize(lightDir.xz);
    float sunAlignment = max(dot(viewXZ, -lightXZ), 0.0);
    float sunRoad = pow(sunAlignment, 4.0); // Tightly bounds the glitter path

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(norm, halfwayDir), 0.0);

    // Smooth specular highlight bounded by the directional sun road
    float specBroad = pow(NdotH, 64.0) * 0.30;
    float specSharp = pow(NdotH, 256.0) * 1.40;
    vec3 sunSpecular = (specBroad + specSharp) * moonLightColor * sunRoad * (fresnel * 1.2 + 0.15);

    vec3 finalColor = waterSurface + sunSpecular;

    // -----------------------------------------------------------------------
    // 5. Dynamic Shoreline Campfire Warm Specular Reflection
    // -----------------------------------------------------------------------
    if (hasCampfire)
    {
        vec3 fireDir = normalize(campfirePos - FragPos);
        float fireDist = length(campfirePos - FragPos);
        float fireAtten = 1.0 / (1.0 + 0.12 * fireDist + 0.035 * (fireDist * fireDist));

        vec3 fireHalfway = normalize(fireDir + viewDir);
        float fireNdotH = max(dot(norm, fireHalfway), 0.0);

        float fireSpec = pow(fireNdotH, 48.0) * 1.80;
        vec3 fireSpecular = fireSpec * campfireColor * 2.2 * fireAtten * campfireIntensity;

        float fireDiff = max(dot(norm, fireDir), 0.0);
        vec3 fireDiffuse = fireDiff * campfireColor * 0.25 * fireAtten * campfireIntensity;

        finalColor += fireSpecular + fireDiffuse;
    }

    // -----------------------------------------------------------------------
    // 6. Dynamic Autumn Dock Lantern Warm Specular Reflection
    // -----------------------------------------------------------------------
    if (hasLantern)
    {
        vec3 lantDir = normalize(lanternPos - FragPos);
        float lantDist = length(lanternPos - FragPos);
        float lantAtten = 1.0 / (1.0 + 0.14 * lantDist + 0.045 * (lantDist * lantDist));

        vec3 lantHalfway = normalize(lantDir + viewDir);
        float lantNdotH = max(dot(norm, lantHalfway), 0.0);

        float lantSpec = pow(lantNdotH, 48.0) * 1.60;
        vec3 lantSpecular = lantSpec * lanternColor * 2.0 * lantAtten * lanternIntensity;

        float lantDiff = max(dot(norm, lantDir), 0.0);
        vec3 lantDiffuse = lantDiff * lanternColor * 0.20 * lantAtten * lanternIntensity;

        finalColor += lantSpecular + lantDiffuse;
    }

    // -----------------------------------------------------------------------
    // 7. Delicate Shoreline Foam & Waterline Blend
    // -----------------------------------------------------------------------
    // Subtle foam only at the very edge touching the shore bank
    float foamLine = smoothstep(0.95, 0.995, ShoreDist);
    vec3 foamColor = vec3(0.68, 0.78, 0.82) * (ambientColor * 1.6 + moonLightColor * 0.4);
    finalColor = mix(finalColor, foamColor, foamLine * 0.25);

    // Realistic water transparency:
    // Shallow shores: clear and transparent (alpha ~0.60) merging softly with the shore
    // Deep center: rich, deep alpine lake (alpha ~0.94)
    float alpha = mix(0.60, 0.94, smoothstep(0.0, 0.35, depthFactor));

    FragColor = vec4(finalColor, alpha);
}

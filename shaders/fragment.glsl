#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 vertexColor;
in vec4 FragPosLightSpace;

out vec4 FragColor;

// Lighting uniforms
uniform vec3 viewPos;
uniform vec3 moonLightDir;    // Direction pointing towards the moon
uniform vec3 moonLightColor;  // Pale cool moonlight
uniform vec3 ambientColor;    // Midnight ambient

// Dynamic Shadow Map Uniforms
uniform sampler2D shadowMap;
uniform bool shadowsEnabled;

// Material properties
uniform bool isEmissive;
uniform vec3 emissionColor;
uniform vec3 objectColor;     // Custom color override for objects
uniform float shininess;
uniform float materialAlpha;
uniform bool isGlass;
uniform bool isGoldCoin;

// Point Light for Bioluminescent Cave Interior
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

uniform PointLight caveLight;
uniform bool useCaveLight;

uniform PointLight fireLight;
uniform bool useFireLight;

uniform PointLight lanternLight;
uniform bool useLanternLight;

uniform PointLight fireplaceLight;
uniform bool useFireplaceLight;

// Localized Point Light for Glowing Gold Coins & Treasure Hoard
uniform PointLight treasureLight;
uniform bool useTreasureLight;

// Moving Point Light for Tourist Train Passenger Coach Windows & Interior Glow
uniform PointLight trainCoachLight;
uniform bool useTrainCoachLight;

// Dynamic Moving Spot Light (Tourist Train Locomotive Headlight Beam)
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float cutOff;       // cos(inner angle)
    float outerCutOff;  // cos(outer angle)
    float constant;
    float linear;
    float quadratic;
};

uniform SpotLight trainSpotLight;
uniform bool useTrainSpotLight;

// Intrinsic self-illumination for gleaming gold coins
uniform float selfIllumination;

// Special celestial rendering uniforms
uniform bool isMoon;
uniform bool isMoonGlow;
uniform bool isSun;
uniform bool isSunGlow;
uniform float sunsetFactor;
uniform float celestialAlpha;
uniform float time;
uniform bool disableLighting; // When true, outputs unlit base colors with no shading

// Volumetric 3D Cloud Uniforms
uniform bool isCloud;
uniform vec3 cloudTopColor;
uniform vec3 cloudBottomColor;

// ---------------------------------------------------------------------------
// Realistic 2D Texture Samplers & Triplanar Mapping
// ---------------------------------------------------------------------------
uniform sampler2D texGrass;
uniform sampler2D texRock;
uniform sampler2D texWood;
uniform sampler2D texCobblestone;
uniform sampler2D texRoof;
uniform sampler2D texMetal;
uniform sampler2D texBark;
uniform sampler2D texLeaf;
uniform int textureMode;       // 0 = none, 1 = grass, 2 = rock, 3 = wood, etc.
uniform float textureTiling;
uniform bool texturesEnabled;

vec3 sampleTriplanarTexture(sampler2D tex, vec3 p, vec3 n, float tiling)
{
    vec3 blend = abs(n);
    blend = pow(blend, vec3(4.0));
    blend /= (blend.x + blend.y + blend.z + 0.00001);

    vec2 uvX = p.zy * tiling;
    vec2 uvY = p.xz * tiling;
    vec2 uvZ = p.xy * tiling;

    vec3 colX = texture(tex, uvX).rgb;
    vec3 colY = texture(tex, uvY).rgb;
    vec3 colZ = texture(tex, uvZ).rgb;

    return colX * blend.x + colY * blend.y + colZ * blend.z;
}

// ---------------------------------------------------------------------------
// 3D Procedural Noise for Realistic Lunar Surface (Maria & Craters)
// ---------------------------------------------------------------------------
float hash(vec3 p)
{
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 x)
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f); // Smooth cubic hermite interpolation

    return mix(
        mix(mix(hash(i + vec3(0,0,0)), hash(i + vec3(1,0,0)), f.x),
            mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash(i + vec3(0,0,1)), hash(i + vec3(1,0,1)), f.x),
            mix(hash(i + vec3(0,1,1)), hash(i + vec3(1,1,1)), f.x), f.y),
        f.z
    );
}

float fbm(vec3 p)
{
    float f = 0.0;
    f += 0.5000 * noise(p * 2.8);
    f += 0.2500 * noise(p * 5.6);
    f += 0.1250 * noise(p * 11.2);
    f += 0.0625 * noise(p * 22.4);
    return f;
}

// ---------------------------------------------------------------------------
// Dynamic Directional Shadow Calculation (Percentage-Closer Filtering - PCF)
// ---------------------------------------------------------------------------
float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Out of bounds or behind far plane: no shadow
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }

    // Adaptive slope bias to prevent shadow acne while keeping shadows tight
    float bias = max(0.0035 * (1.0 - dot(normal, lightDir)), 0.0008);

    // 9-tap Percentage-Closer Filtering (PCF) for smooth, soft shadow edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

// ---------------------------------------------------------------------------
// Analytical Ray Tracing System for Gold Coins & Treasure Hoard
// (Kept in dedicated module for clean separation and explainability)
// ---------------------------------------------------------------------------
#include "raytracing_coins.glsl"

void main()

{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // -----------------------------------------------------------------------
    // 1. Realistic Soft Atmospheric Moon Halo (Radial Falloff)
    // -----------------------------------------------------------------------
    if (isMoonGlow)
    {
        vec2 coord = (length(vertexColor.xy) > 0.001) ? vertexColor.xy : Normal.xy;
        float dist = length(coord);
        if (dist > 1.0) discard;

        // Multi-tier smooth exponential atmospheric glow
        float innerGlow = exp(-dist * 4.0);
        float outerHaze = exp(-dist * 1.8) * 0.45;
        float rimFade = smoothstep(1.0, 0.35, dist);
        float effectiveAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        float alpha = (innerGlow + outerHaze) * rimFade * 0.85 * effectiveAlpha;

        vec3 glowColor = vec3(0.65, 0.78, 0.95); // Cool ethereal blue-white haze
        FragColor = vec4(glowColor * alpha, alpha);
        return;
    }

    // -----------------------------------------------------------------------
    // 2. Realistic 3D Moon Surface (Spherical Shading + Lunar Maria & Craters)
    // -----------------------------------------------------------------------
    if (isMoon)
    {
        // Sample seamless 3D fractal noise along the sphere's surface normal
        float maria = fbm(norm * 3.2);
        float craters = fbm(norm * 12.0);

        // Palette: Bright lunar highlands vs Dark lunar basalt plains (Maria)
        vec3 highlandColor = vec3(0.96, 0.97, 0.94); // Pale silvery ivory
        vec3 mariaColor    = vec3(0.62, 0.66, 0.70); // Darker basalt gray
        vec3 craterDark    = vec3(0.52, 0.55, 0.58);

        // Blend lunar surface features
        vec3 moonColor = mix(mariaColor, highlandColor, smoothstep(0.38, 0.62, maria));
        moonColor = mix(moonColor, craterDark, smoothstep(0.70, 0.85, craters) * 0.35);

        // 3D Spherical Depth (Limb Darkening & subtle celestial hemisphere gradient)
        float NdotV = max(dot(norm, viewDir), 0.0);
        float limbDarkening = pow(NdotV, 0.35); // Soft roll-off towards the edges

        // Subtle 3D spherical directional depth across the face of the moon
        vec3 moonSunDir = normalize(vec3(0.35, 0.45, 0.82));
        float hemisphere3D = max(dot(norm, moonSunDir), 0.0) * 0.28 + 0.72;

        vec3 finalMoon = moonColor * limbDarkening * hemisphere3D * 1.15;
        float effectiveAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        FragColor = vec4(finalMoon, effectiveAlpha);
        return;
    }

    // -----------------------------------------------------------------------
    // 3. Realistic Atmospheric Solar Corona & Multi-Tier Aureole
    // -----------------------------------------------------------------------
    if (isSunGlow)
    {
        vec2 coord = (length(vertexColor.xy) > 0.001) ? vertexColor.xy : Normal.xy;
        float dist = length(coord);
        if (dist > 1.0) discard;

        // Multi-tier solar atmospheric aureole:
        // Intense inner core aureole + expanding warm golden corona + soft atmospheric sky scatter
        float innerCorona = exp(-dist * 4.8) * 1.30;
        float middleHalo   = exp(-dist * 2.2) * 0.65;
        float outerHaze   = exp(-dist * 1.1) * 0.30;
        float rimFade     = smoothstep(1.0, 0.30, dist);

        // Dynamic solar rays & coronal streamers
        float angle = atan(coord.y, coord.x);
        float streamers = 0.92 + 0.08 * sin(angle * 10.0 + time * 0.35) * cos(angle * 6.0 - time * 0.20);

        float effectiveAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        float alpha = (innerCorona + middleHalo + outerHaze) * rimFade * streamers * effectiveAlpha;

        // Chromatic temperature gradient from blazing core outward
        // Midday vs Sunset
        vec3 dayCoreGlow   = vec3(1.35, 1.25, 0.98); // Incandescent white-gold
        vec3 dayMidGlow    = vec3(1.10, 0.88, 0.48); // Radiant sunny gold
        vec3 dayOuterGlow  = vec3(0.92, 0.75, 0.40); // Soft solar haze

        vec3 sunsetCoreGlow  = vec3(1.35, 0.90, 0.45); // Deep fiery gold
        vec3 sunsetMidGlow   = vec3(1.15, 0.52, 0.18); // Rich amber/peach
        vec3 sunsetOuterGlow = vec3(0.90, 0.32, 0.08); // Twilight crimson

        vec3 coreGlow  = mix(dayCoreGlow,  sunsetCoreGlow,  sunsetFactor);
        vec3 midGlow   = mix(dayMidGlow,   sunsetMidGlow,   sunsetFactor);
        vec3 outerGlow = mix(dayOuterGlow, sunsetOuterGlow, sunsetFactor);

        vec3 glowCol = mix(coreGlow, midGlow, smoothstep(0.0, 0.45, dist));
        glowCol = mix(glowCol, outerGlow, smoothstep(0.45, 0.90, dist));

        FragColor = vec4(glowCol * alpha, alpha);
        return;
    }

    // -----------------------------------------------------------------------
    // 4. Realistic 3D Solar Photosphere (3D Spherical Shading & Solar Granulation)
    // -----------------------------------------------------------------------
    if (isSun)
    {
        // 3D Spherical Solar Granulation (convective plasma bubbles across sphere surface)
        vec3 pNorm = norm * 4.5 + vec3(time * 0.03, -time * 0.02, time * 0.025);
        float granulation = fbm(pNorm);
        float finePlasma = noise(norm * 14.0 + vec3(-time * 0.06, time * 0.05, 0.0));
        float plasmaTurbulence = granulation * 0.70 + finePlasma * 0.30;

        // Eddington solar limb darkening:
        // Looking directly at center (dot(norm, viewDir) ≈ 1) reveals deep incandescent core
        // Looking towards the rim (dot(norm, viewDir) → 0) glances through cooler upper layers
        float NdotV = max(dot(norm, viewDir), 0.0);
        float limbDarkening = pow(NdotV, 0.42);

        // Midday Palette: Incandescent blazing core -> Golden yellow photosphere -> Warm amber limb
        vec3 dayCore  = vec3(1.35, 1.30, 1.12);
        vec3 dayMid   = vec3(1.15, 0.95, 0.58);
        vec3 dayLimb  = vec3(1.00, 0.70, 0.28);

        // Sunset Palette: Radiant golden core -> Fiery orange photosphere -> Deep crimson-amber limb
        vec3 sunsetCore = vec3(1.30, 0.95, 0.55);
        vec3 sunsetMid  = vec3(1.18, 0.55, 0.18);
        vec3 sunsetLimb = vec3(0.92, 0.30, 0.08);

        vec3 coreColor = mix(dayCore, sunsetCore, sunsetFactor);
        vec3 midColor  = mix(dayMid,  sunsetMid,  sunsetFactor);
        vec3 limbColor = mix(dayLimb, sunsetLimb, sunsetFactor);

        // 3D Photospheric color gradient across spherical depth
        vec3 sunColor = mix(limbColor, midColor, smoothstep(0.0, 0.52, limbDarkening));
        sunColor = mix(sunColor, coreColor, smoothstep(0.52, 1.0, limbDarkening));

        // Modulate with convective plasma granulation cells
        sunColor *= (0.91 + 0.18 * plasmaTurbulence);

        // Chromosphere / Coronal rim fringe at grazing edge (Fresnel)
        float rimFresnel = pow(1.0 - NdotV, 3.2);
        vec3 rimColor = mix(vec3(1.05, 0.85, 0.40), vec3(1.05, 0.45, 0.12), sunsetFactor);
        sunColor += rimColor * rimFresnel * 0.85;

        float effectiveAlpha = (celestialAlpha > 0.0) ? celestialAlpha : 1.0;
        FragColor = vec4(sunColor, effectiveAlpha);
        return;
    }

    // -----------------------------------------------------------------------
    // Volumetric 3D Cloud Shading (Wrap diffuse & soft ash ambient underside)
    // -----------------------------------------------------------------------
    if (isCloud)
    {
        vec3 cLightDir = normalize(moonLightDir);
        // Soft wrap diffuse for fluffy volumetric clouds (smooth gradient across sphere)
        float wrapDiff = clamp(dot(norm, cLightDir) * 0.5 + 0.5, 0.0, 1.0);
        float upFactor = clamp(norm.y * 0.5 + 0.5, 0.0, 1.0);
        
        // Blend between sunlit top and ash-toned ambient underside
        float lightFactor = clamp(wrapDiff * 0.65 + upFactor * 0.35, 0.0, 1.0);
        vec3 cloudCol = mix(cloudBottomColor, cloudTopColor, lightFactor);
        
        // Soft forward rim scattering when looking near cloud edges
        float rim = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);
        cloudCol += moonLightColor * rim * 0.15;
        
        FragColor = vec4(cloudCol, 1.0);
        return;
    }

    vec3 surfaceCol = (length(objectColor) > 0.001) ? objectColor : vertexColor;


    // -----------------------------------------------------------------------
    // Realistic Material Texture Modulation
    // -----------------------------------------------------------------------
    if (texturesEnabled && textureMode > 0)
    {
        vec3 texDetail = vec3(1.0);
        float tiling = (textureTiling > 0.001) ? textureTiling : 0.5;

        if (textureMode == 1)      texDetail = sampleTriplanarTexture(texGrass, FragPos, norm, tiling * 0.35);
        else if (textureMode == 2) texDetail = sampleTriplanarTexture(texRock, FragPos, norm, tiling * 0.22);
        else if (textureMode == 3) texDetail = sampleTriplanarTexture(texWood, FragPos, norm, tiling * 0.75);
        else if (textureMode == 4) texDetail = sampleTriplanarTexture(texCobblestone, FragPos, norm, tiling * 0.45);
        else if (textureMode == 5) texDetail = sampleTriplanarTexture(texRoof, FragPos, norm, tiling * 0.55);
        else if (textureMode == 6) texDetail = sampleTriplanarTexture(texMetal, FragPos, norm, tiling * 0.65);
        else if (textureMode == 7) texDetail = sampleTriplanarTexture(texBark, FragPos, norm, tiling * 0.50);
        else if (textureMode == 8) texDetail = sampleTriplanarTexture(texLeaf, FragPos, norm, tiling * 0.90);

        // Blend texture albedo with base mesh/vertex color while preserving luminosity
        surfaceCol = surfaceCol * texDetail * 1.35;
    }

    // -----------------------------------------------------------------------
    // Flat / Unlit Mode (Stop all lighting and shading calculations)
    // -----------------------------------------------------------------------
    if (disableLighting)
    {
        FragColor = vec4(surfaceCol, 1.0);
        return;
    }

    // -----------------------------------------------------------------------
    // 3. Generic Emissive Objects (Sky Backdrop, glowing windows, etc.)
    // -----------------------------------------------------------------------
    if (isEmissive)
    {
        FragColor = vec4(surfaceCol * emissionColor, 1.0);
        return;
    }

    // -----------------------------------------------------------------------
    // 4. Standard Blinn-Phong Shading for Environment Objects
    // -----------------------------------------------------------------------
    vec3 lightDir = normalize(moonLightDir);

    // Ambient
    vec3 ambient = ambientColor * surfaceCol;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * moonLightColor * surfaceCol;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess > 0.0 ? shininess : 32.0);
    vec3 specular = spec * moonLightColor * 0.35;

    float shadow = 0.0;
    if (shadowsEnabled)
    {
        shadow = calculateShadow(FragPosLightSpace, norm, lightDir);
    }

    vec3 selfGlow = surfaceCol * selfIllumination;
    // Shadows reduce direct sunlight/moonlight; ambient lighting remains fully active
    vec3 result = ambient + selfGlow + (1.0 - shadow * 0.88) * (diffuse + specular);

    // Point Light Contribution (Flaming Torch / Entrance Stand)
    // "the lighting of it is very bright. reduce the raduis it spreads its illumination."
    if (useCaveLight)
    {
        vec3 pLightDir = normalize(caveLight.position - FragPos);
        float pDist = length(caveLight.position - FragPos);

        // Strictly localized radius: smooth cutoff to 0 beyond 5.0 meters
        if (pDist < 5.0)
        {
            float cutoff = smoothstep(5.0, 2.2, pDist);
            float atten = 1.0 / (caveLight.constant + caveLight.linear * pDist + caveLight.quadratic * (pDist * pDist));

            float pDiff = max(dot(norm, pLightDir), 0.0);
            vec3 pDiffuse = pDiff * caveLight.color * surfaceCol;

            vec3 pHalfway = normalize(pLightDir + viewDir);
            float pSpec = pow(max(dot(norm, pHalfway), 0.0), shininess > 0.0 ? shininess : 32.0);
            vec3 pSpecular = pSpec * caveLight.color * 0.28;

            result += (pDiffuse + pSpecular) * (atten * cutoff) * caveLight.intensity;
        }
    }

    // Point Light Contribution (Warm Campfire Glow)
    if (useFireLight)
    {
        vec3 fLightDir = normalize(fireLight.position - FragPos);
        float fDist = length(fireLight.position - FragPos);
        float fAtten = 1.0 / (fireLight.constant + fireLight.linear * fDist + fireLight.quadratic * (fDist * fDist));

        float fDiff = max(dot(norm, fLightDir), 0.0);
        vec3 fDiffuse = fDiff * fireLight.color * surfaceCol;

        vec3 fHalfway = normalize(fLightDir + viewDir);
        float fSpec = pow(max(dot(norm, fHalfway), 0.0), shininess > 0.0 ? shininess : 24.0);
        vec3 fSpecular = fSpec * fireLight.color * 0.40;

        result += (fDiffuse + fSpecular) * fAtten * fireLight.intensity;
    }

    // Point Light Contribution (Warm Autumn Lantern Glow)
    if (useLanternLight)
    {
        vec3 lLightDir = normalize(lanternLight.position - FragPos);
        float lDist = length(lanternLight.position - FragPos);
        float lAtten = 1.0 / (lanternLight.constant + lanternLight.linear * lDist + lanternLight.quadratic * (lDist * lDist));

        float lDiff = max(dot(norm, lLightDir), 0.0);
        vec3 lDiffuse = lDiff * lanternLight.color * surfaceCol;

        vec3 lHalfway = normalize(lLightDir + viewDir);
        float lSpec = pow(max(dot(norm, lHalfway), 0.0), shininess > 0.0 ? shininess : 20.0);
        vec3 lSpecular = lSpec * lanternLight.color * 0.35;

        result += (lDiffuse + lSpecular) * lAtten * lanternLight.intensity;
    }

    // Point Light Contribution (Cabin Fireplace Hearth Glow)
    if (useFireplaceLight)
    {
        vec3 fpLightDir = normalize(fireplaceLight.position - FragPos);
        float fpDist = length(fireplaceLight.position - FragPos);
        float fpAtten = 1.0 / (fireplaceLight.constant + fireplaceLight.linear * fpDist + fireplaceLight.quadratic * (fpDist * fpDist));

        float fpDiff = max(dot(norm, fpLightDir), 0.0);
        vec3 fpDiffuse = fpDiff * fireplaceLight.color * surfaceCol;

        vec3 fpHalfway = normalize(fpLightDir + viewDir);
        float fpSpec = pow(max(dot(norm, fpHalfway), 0.0), shininess > 0.0 ? shininess : 24.0);
        vec3 fpSpecular = fpSpec * fireplaceLight.color * 0.40;

        result += (fpDiffuse + fpSpecular) * fpAtten * fireplaceLight.intensity;
    }

    // Point Light Contribution (Localized Glowing Treasure Hoard & Gold Coins)
    // "they shouldnt light up the whole cave, only the area around them"
    if (useTreasureLight)
    {
        vec3 tLightDir = normalize(treasureLight.position - FragPos);
        float tDist = length(treasureLight.position - FragPos);

        // Localized radius: illuminates the treasure sanctum, wall sconces, chests, and coins
        if (tDist < 6.5)
        {
            float atten = 1.0 / (treasureLight.constant + treasureLight.linear * tDist + treasureLight.quadratic * (tDist * tDist));
            float cutoff = smoothstep(6.5, 2.6, tDist); // Smoothly tapers to 0 between 2.6m and 6.5m

            float tDiff = max(dot(norm, tLightDir), 0.0);
            vec3 tDiffuse = tDiff * treasureLight.color * surfaceCol;

            vec3 tHalfway = normalize(tLightDir + viewDir);
            float tSpec = pow(max(dot(norm, tHalfway), 0.0), shininess > 0.0 ? shininess : 32.0);
            vec3 tSpecular = tSpec * treasureLight.color * 0.70; // Brilliant golden metallic sheen

            result += (tDiffuse + tSpecular) * (atten * cutoff) * treasureLight.intensity;
        }
    }

    // -----------------------------------------------------------------------
    // Point Light Contribution (Moving Tourist Train Coach Interior & Windows Warm Glow)
    // -----------------------------------------------------------------------
    if (useTrainCoachLight)
    {
        vec3 tcLightDir = normalize(trainCoachLight.position - FragPos);
        float tcDist = length(trainCoachLight.position - FragPos);
        if (tcDist < 16.0)
        {
            float tcAtten = 1.0 / (trainCoachLight.constant + trainCoachLight.linear * tcDist + trainCoachLight.quadratic * (tcDist * tcDist));
            float tcCutoff = smoothstep(16.0, 3.0, tcDist);
            float tcDiff = max(dot(norm, tcLightDir), 0.0);
            vec3 tcDiffuse = tcDiff * trainCoachLight.color * surfaceCol;
            vec3 tcHalfway = normalize(tcLightDir + viewDir);
            float tcSpec = pow(max(dot(norm, tcHalfway), 0.0), shininess > 0.0 ? shininess : 28.0);
            vec3 tcSpecular = tcSpec * trainCoachLight.color * 0.35;
            result += (tcDiffuse + tcSpecular) * (tcAtten * tcCutoff) * trainCoachLight.intensity;
        }
    }

    // -----------------------------------------------------------------------
    // Spot Light Contribution (Moving Tourist Train Headlight Focused Cone)
    // -----------------------------------------------------------------------
    if (useTrainSpotLight)
    {
        vec3 sLightDir = normalize(trainSpotLight.position - FragPos);
        float sDist = length(trainSpotLight.position - FragPos);

        // Angle between light-to-fragment vector and the spotlight beam direction
        float theta = dot(-sLightDir, normalize(trainSpotLight.direction));

        // Soft outer edge falloff using epsilon
        float epsilon = trainSpotLight.cutOff - trainSpotLight.outerCutOff;
        float spotIntensity = clamp((theta - trainSpotLight.outerCutOff) / (epsilon > 0.0001 ? epsilon : 0.0001), 0.0, 1.0);

        if (spotIntensity > 0.0)
        {
            float sAtten = 1.0 / (trainSpotLight.constant + trainSpotLight.linear * sDist + trainSpotLight.quadratic * (sDist * sDist));

            // Diffuse component
            float sDiff = max(dot(norm, sLightDir), 0.0);
            vec3 sDiffuse = sDiff * trainSpotLight.color * surfaceCol;

            // Specular component (Blinn-Phong)
            vec3 sHalfway = normalize(sLightDir + viewDir);
            float sSpec = pow(max(dot(norm, sHalfway), 0.0), shininess > 0.0 ? shininess : 32.0);
            vec3 sSpecular = sSpec * trainSpotLight.color * 0.45;

            result += (sDiffuse + sSpecular) * (sAtten * spotIntensity) * trainSpotLight.intensity;
        }
    }

    if (isGlass)
    {
        // Realistic glass Fresnel sheen & high-gloss specular highlight
        float NdotV = max(dot(norm, viewDir), 0.0);
        float glassFresnel = pow(1.0 - NdotV, 3.5) * 0.45;
        result = mix(result, vec3(0.85, 0.94, 1.0), glassFresnel);
    }

    // -----------------------------------------------------------------------
    // Real-Time Ray Traced Metallic Coin Shading (from raytracing_coins.glsl)
    // -----------------------------------------------------------------------
    if (isGoldCoin)
    {
        result = shadeRayTracedCoin(FragPos, norm, viewDir, surfaceCol, result, treasureLight.position, enableCoinRayTracing);
    }

    float outAlpha = (materialAlpha > 0.001) ? materialAlpha : 1.0;

    FragColor = vec4(result, outAlpha);
}


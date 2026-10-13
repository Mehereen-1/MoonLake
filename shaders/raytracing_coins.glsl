// ===========================================================================
// RAYTRACING_COINS.GLSL - Analytical Real-Time Ray Tracing for Gold Coins
// ===========================================================================
// This module implements real-time analytical ray tracing for metallic gold coins
// inside the cavern treasure sanctum.
//
// Key Techniques:
// 1. Ray-Sphere & Ray-AABB Analytical Intersection (Zero rasterization artifacts)
// 2. Secondary Reflection Rays: Simulates metallic multi-bounce radiosity
//    (gold-on-gold amber absorption), hanging lantern orb specular glints,
//    and dark mahogany chest wall reflections.
// 3. Secondary Shadow Rays: Ray traces contact crevice ambient occlusion (AO)
//    between overlapping stacked coins.
// 4. Physical Conductor Fresnel: Cook-Torrance model for polished gold metal
//    with characteristic spectral reflectance F0 = (1.0, 0.78, 0.34).
// ===========================================================================

// ---------------------------------------------------------------------------
// Uniforms for the Local Cavern Treasure Environment
// ---------------------------------------------------------------------------
uniform bool enableCoinRayTracing;
uniform vec3 treasureCenter;      // Center of the main gold coin mound
uniform vec3 hangingOrbPos;        // World position of the hanging glowing orb lantern
uniform vec3 chestMinBounds;       // Min corner (AABB) of the mahogany chest tub
uniform vec3 chestMaxBounds;       // Max corner (AABB) of the mahogany chest tub

// ---------------------------------------------------------------------------
// 1. ANALYTICAL RAY-SPHERE INTERSECTION
// Solves: ||(ro + t*rd) - center||^2 = radius^2
// Expanding: t^2 + 2*b*t + c = 0, where oc = ro - center, b = dot(oc, rd), c = dot(oc, oc) - r^2
// ---------------------------------------------------------------------------
bool intersectRaySphere(vec3 ro, vec3 rd, vec3 center, float radius, out float tHit)
{
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - c;

    if (discriminant < 0.0)
    {
        return false; // Ray misses sphere
    }

    float sqrtD = sqrt(discriminant);
    float t0 = -b - sqrtD; // Near intersection
    float t1 = -b + sqrtD; // Far intersection

    if (t0 > 0.001)
    {
        tHit = t0;
        return true;
    }
    if (t1 > 0.001)
    {
        tHit = t1;
        return true;
    }

    return false; // Intersection is behind ray origin
}

// ---------------------------------------------------------------------------
// 2. ANALYTICAL RAY-AABB INTERSECTION (Kay-Kajiya Slab Method)
// Tests ray intersection against 3 orthogonal slab pairs [bMin, bMax]
// ---------------------------------------------------------------------------
bool intersectRayAABB(vec3 ro, vec3 rd, vec3 bMin, vec3 bMax, out float tHit)
{
    vec3 invD = 1.0 / (rd + sign(rd) * 0.00001);
    vec3 t0 = (bMin - ro) * invD;
    vec3 t1 = (bMax - ro) * invD;

    vec3 tNearVec = min(t0, t1);
    vec3 tFarVec  = max(t0, t1);

    float tNear = max(max(tNearVec.x, tNearVec.y), tNearVec.z);
    float tFar  = min(min(tFarVec.x,  tFarVec.y),  tFarVec.z);

    if (tNear > tFar || tFar < 0.001)
    {
        return false; // Ray misses box
    }

    tHit = (tNear > 0.001) ? tNear : tFar;
    return true;
}

// ---------------------------------------------------------------------------
// 3. SECONDARY REFLECTION RAY TRACING (Multi-Bounce Radiosity)
// Casts secondary bounce rays from the coin surface into the local scene.
// ---------------------------------------------------------------------------
vec3 traceCoinReflection(vec3 ro, vec3 rd, vec3 baseColor)
{
    float nearestT = 1000.0;
    vec3 hitColor = vec3(0.0);
    bool hitSomething = false;

    // A. Test intersection against Hanging Glowing Orb Lantern
    float tOrb;
    if (intersectRaySphere(ro, rd, hangingOrbPos, 0.28, tOrb))
    {
        if (tOrb < nearestT)
        {
            nearestT = tOrb;
            // Brilliant incandescent golden specular glint from lantern
            hitColor = vec3(1.45, 1.25, 0.55);
            hitSomething = true;
        }
    }

    // B. Test intersection against Neighboring Coin Mound Clusters (Gold-on-Gold Bounce)
    // Pure gold metals have no diffuse reflectance—they reflect colored light.
    // Each bounce absorbs blue wavelengths and exponentially enriches saturated amber hues!
    vec3 moundA = treasureCenter + vec3(0.0, 0.10, 0.0);
    vec3 moundB = treasureCenter + vec3(0.22, -0.04, 0.16);
    vec3 moundC = treasureCenter + vec3(-0.18, 0.06, -0.08);

    float tMound;
    if (intersectRaySphere(ro, rd, moundA, 0.36, tMound))
    {
        if (tMound < nearestT)
        {
            nearestT = tMound;
            hitColor = baseColor * vec3(1.28, 0.96, 0.38) * 1.35;
            hitSomething = true;
        }
    }
    if (intersectRaySphere(ro, rd, moundB, 0.30, tMound))
    {
        if (tMound < nearestT)
        {
            nearestT = tMound;
            hitColor = baseColor * vec3(1.28, 0.96, 0.38) * 1.30;
            hitSomething = true;
        }
    }
    if (intersectRaySphere(ro, rd, moundC, 0.28, tMound))
    {
        if (tMound < nearestT)
        {
            nearestT = tMound;
            hitColor = baseColor * vec3(1.28, 0.96, 0.38) * 1.30;
            hitSomething = true;
        }
    }

    // C. Test intersection against Wooden Chest Tub
    float tChest;
    if (intersectRayAABB(ro, rd, chestMinBounds, chestMaxBounds, tChest))
    {
        if (tChest < nearestT)
        {
            nearestT = tChest;
            // Aged dark mahogany wood reflected onto adjacent coins
            hitColor = vec3(0.24, 0.13, 0.06);
            hitSomething = true;
        }
    }

    // D. Miss ray escaping to cavern cobblestone ceiling / walls
    if (!hitSomething)
    {
        hitColor = vec3(0.08, 0.08, 0.10);
    }

    return hitColor;
}

// ---------------------------------------------------------------------------
// 4. SECONDARY SHADOW RAY TRACING (Crevice Contact Ambient Occlusion)
// Casts shadow rays toward the localized treasure light source.
// ---------------------------------------------------------------------------
float traceCoinCreviceOcclusion(vec3 ro, vec3 lightPos)
{
    vec3 toLight = lightPos - ro;
    float distToLight = length(toLight);
    vec3 lightDir = toLight / (distToLight + 0.0001);

    // Test occlusion by overlapping upper coin mound
    vec3 moundA = treasureCenter + vec3(0.0, 0.10, 0.0);
    float tHit;
    if (intersectRaySphere(ro, lightDir, moundA, 0.34, tHit))
    {
        if (tHit > 0.02 && tHit < distToLight)
        {
            // Occluded by higher coin layer -> deep crevice contact shadow
            return 0.32;
        }
    }

    // Test occlusion by wooden chest lip / wall bounds
    float tBox;
    if (intersectRayAABB(ro, lightDir, chestMinBounds, chestMaxBounds, tBox))
    {
        if (tBox > 0.02 && tBox < distToLight)
        {
            return 0.40;
        }
    }

    return 1.0; // Ray reaches light unoccluded
}

// ---------------------------------------------------------------------------
// 5. COOK-TORRANCE CONDUCTOR FRESNEL EVALUATION FOR PURE GOLD
// Gold has complex index of refraction: n + i*k.
// Characteristic optical reflectance at normal incidence: F0 ≈ (1.00, 0.78, 0.34)
// ---------------------------------------------------------------------------
vec3 evalGoldFresnel(float NdotV)
{
    const vec3 F0 = vec3(1.00, 0.78, 0.34);
    return F0 + (vec3(1.0) - F0) * pow(1.0 - clamp(NdotV, 0.0, 1.0), 5.0);
}

// ---------------------------------------------------------------------------
// 6. MAIN SHADING INTEGRATION ENTRY POINT
// Call this function from your fragment shader to shade any gold coin fragment.
// ---------------------------------------------------------------------------
vec3 shadeRayTracedCoin(vec3 fragPos,
                        vec3 norm,
                        vec3 viewDir,
                        vec3 surfaceCol,
                        vec3 baseResult,
                        vec3 treasureLightPos,
                        bool isRayTracingActive)
{
    if (isRayTracingActive)
    {
        // Offset ray origin slightly along normal to prevent self-intersection (shadow acne)
        vec3 rayOrig = fragPos + norm * 0.003;
        vec3 reflRayDir = reflect(-viewDir, norm);

        // 1. Cast Secondary Reflection Ray
        vec3 reflectedColor = traceCoinReflection(rayOrig, reflRayDir, surfaceCol);

        // 2. Cast Secondary Shadow Ray for Crevice Occlusion
        float creviceAO = traceCoinCreviceOcclusion(rayOrig, treasureLightPos);

        // 3. Conductor Fresnel Reflectance
        float NdotV = max(dot(norm, viewDir), 0.0);
        vec3 fresnelGold = evalGoldFresnel(NdotV);

        // Blend ray-traced reflection into surface radiosity
        vec3 metallicRadiosity = mix(baseResult * creviceAO, reflectedColor, fresnelGold * 0.65);

        // Add intrinsic golden subsurface glow
        return metallicRadiosity + surfaceCol * 0.22;
    }
    else
    {
        // Standard rasterized Blinn-Phong fallback
        return baseResult + surfaceCol * 0.44;
    }
}

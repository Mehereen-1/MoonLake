#ifndef REALISTIC_TREE_MESH_H
#define REALISTIC_TREE_MESH_H

#include "Mesh.h"
#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// RealisticTreeMesh Class
// Procedural organic tree mesh featuring:
//   - Botanically accurate hierarchical branching (trunk, primary boughs,
//     secondary branches, and tertiary twigs) with tapered cylindrical segments.
//   - Root flare and seamless branch joint nodes.
//   - Dense, realistic foliage canopy with volumetric leaf cluster cores
//     and multi-angled creased leaf blades catching light dynamically.
//   - Multi-tone vertex color gradients (sunlit spring lime, lush chlorophyll
//     summer green, deep forest emerald, weathered bark umbers).
//   - 4 organic structural variants + multiple seasonal color palettes.
// ---------------------------------------------------------------------------
class RealisticTreeMesh : public Mesh
{
public:
    // variant: 0 = Image match (curved trunk with prominent low-right bough)
    //          1 = Majestic spreading crown
    //          2 = Graceful sweeping/weeping boughs
    //          3 = Twin-stem woodland oak
    // paletteType: 0 = Lush Summer Green (Default)
    //              1 = Autumn Amber Orange
    //              2 = Autumn Golden Saffron
    //              3 = Autumn Crimson Red
    //              4 = Autumn Copper Ochre
    RealisticTreeMesh(int variant = 0, int paletteType = 0);
    virtual ~RealisticTreeMesh() = default;

private:
    void buildMesh(int variant, int paletteType);
};

#endif // REALISTIC_TREE_MESH_H

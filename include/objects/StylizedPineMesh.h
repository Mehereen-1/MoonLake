#ifndef STYLIZED_PINE_MESH_H
#define STYLIZED_PINE_MESH_H

#include "Mesh.h"
#include <glm/glm.hpp>
#include <vector>

#include "Shader.h"

// ---------------------------------------------------------------------------
// StylizedPineMesh Class
// Procedural stylized evergreen / pine tree mesh accurately matching the
// reference art style:
//   - Tiered whorls of stylized fanned evergreen boughs drooping under gravity.
//   - 3D creased needle fans with 3-5 pointed serrated fingertips.
//   - Leaf-fiber texture mapping across the foliage boughs.
//   - Bark texture mapping across the trunk and dead lower twigs.
//   - Vibrant hand-painted color gradients with multi-palette support:
//       0 = Summer Green
//       1 = Autumn Amber Orange
//       2 = Autumn Golden Saffron
//       3 = Autumn Crimson Scarlet
//       4 = Autumn Copper Ochre
//   - Authentic dead / withered bare twigs beneath the lowest foliage tier.
//   - Fluted tapered bark trunk with root flare.
//   - 5 distinct procedural variants directly corresponding to the 5 trees in
//     the reference image.
// ---------------------------------------------------------------------------
class StylizedPineMesh : public Mesh
{
public:
    // variant: 0 to 4 (the 5 tree models in the reference image)
    // paletteType: 0 = Summer Green, 1 = Amber Orange, 2 = Golden Saffron, 3 = Crimson, 4 = Copper Ochre
    StylizedPineMesh(int variant = 0, int paletteType = 0);
    virtual ~StylizedPineMesh() = default;

    void draw(const Shader& shader) const;
    virtual void draw() const override;

private:
    class SubMesh : public Mesh {
    public:
        void init(const std::vector<Vertex>& v, const std::vector<unsigned int>& i) {
            setupMesh(v, i);
        }
    };

    void buildMesh(int variant, int paletteType);

    SubMesh m_trunkMesh;
    SubMesh m_foliageMesh;
};

#endif // STYLIZED_PINE_MESH_H

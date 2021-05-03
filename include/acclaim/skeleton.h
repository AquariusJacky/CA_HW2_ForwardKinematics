#pragma once
#include <string>
#include <vector>

#include "bone.h"
#include "graphics/cylinder.h"
#include "util/filesystem.h"

namespace graphics {
class Program;
}
namespace acclaim {
class Skeleton final {
 public:
    // Root always has index 0
    static constexpr int root_idx() noexcept { return 0; }
    Skeleton(const util::fs::path &file_name, const double _scale) noexcept;
    Skeleton(const Skeleton &) noexcept;
    Skeleton(Skeleton &&) noexcept;

    Skeleton &operator=(const Skeleton &) noexcept;
    Skeleton &operator=(Skeleton &&) noexcept;
    // You need this for alignment otherwise it may crash
    // Ref: https://eigen.tuxfamily.org/dox/group__TopicStructHavingEigenMembers.html
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    // get skeleton's scale
    double getScale() const;
    // get total bones in the skeleton
    int getBoneNum() const;
    // get total movable bones in the skeleton
    int getMovableBoneNum() const;
    // get specific bone by its name
    Bone *getBonePointer(const std::string &name);
    // get specific bone by its index
    Bone *getBonePointer(const int bone_idx);
    // set bone's color (for rendering)
    void setBoneColor(const Eigen::Vector4f &boneColor);
    // set bone's model matrices (for rendering)
    void setModelMatrices();
    // render the bone
    void render(graphics::Program *program);

 private:
    bool readASFFile(const util::fs::path &file_name);
    // *********************************
    // This recursive function traverces skeleton hierarchy
    // and returns a pointer to the bone with index - bIndex
    // ptr should be a pointer to the root node
    // when this function first called
    // *********************************
    Bone *getBone(Bone *ptr, const int bIndex);
    // This function sets sibling or child for parent bone
    // If parent bone does not have a child,
    // then pChild is set as parent's child
    // else pChild is set as a sibling of parents already existing child
    int setChildrenSibling(const int parent, Bone *pChild);
    // Transform the direction vector (dir),
    // which is defined in character's global coordinate system in the ASF file,
    // to local coordinate
    void rotateBone2LocalCoord();
    void computeRotationParent2Child(Bone *parent, Bone *child);
    void computeRotation2ParentCoord(Bone *bone);
    // setup graphics
    void setBoneGraphics();

    double scale = 0.2;
    int movableBones = 1;
    std::vector<Bone> bones = std::vector<Bone>(1);
    std::vector<graphics::Cylinder> bone_graphics;
};
}  // namespace acclaim

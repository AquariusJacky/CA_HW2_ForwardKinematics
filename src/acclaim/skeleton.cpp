#include "acclaim/skeleton.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

#include "util/helper.h"

namespace acclaim {

Skeleton::Skeleton(const util::fs::path &file_name, const double _scale) noexcept : scale(_scale) {
    bones.reserve(64);
    // Initializaton of root bone
    bones[0].name = std::string("root");
    bones[0].idx = root_idx();
    bones[0].parent = nullptr;
    bones[0].sibling = nullptr;
    bones[0].child = nullptr;
    bones[0].dir.setZero();
    bones[0].axis.setZero();
    bones[0].length = 0;
    bones[0].dof = 6;
    bones[0].dofrx = true;
    bones[0].dofry = true;
    bones[0].dofrz = true;
    bones[0].doftx = true;
    bones[0].dofty = true;
    bones[0].doftz = true;
    // build hierarchy and read in each bone's DOF information
    readASFFile(file_name);
    // transform the direction vector for each bone from the world coordinate system
    // to it's local coordinate system
    rotateBone2LocalCoord();
    // Calculate rotation from each bone local coordinate system to the coordinate system of its parent
    // store it in rot_parent_current variable for each bone
    computeRotation2ParentCoord(&bones[0]);

    setBoneGraphics();
}

Skeleton::Skeleton(const Skeleton &other) noexcept
    : scale(other.scale), movableBones(other.movableBones), bones(other.bones), bone_graphics(other.bone_graphics) {
    for (std::size_t i = 0; i < bones.size(); ++i) {
        if (bones[i].parent != nullptr) {
            bones[i].parent = &bones[other.bones[i].parent->idx];
        }
        if (bones[i].child != nullptr) {
            bones[i].child = &bones[other.bones[i].child->idx];
        }
        if (bones[i].sibling != nullptr) {
            bones[i].sibling = &bones[other.bones[i].sibling->idx];
        }
    }
}

Skeleton::Skeleton(Skeleton &&other) noexcept
    : scale(other.scale),
      movableBones(other.movableBones),
      bones(std::move(other.bones)),
      bone_graphics(std::move(other.bone_graphics)) {}

Skeleton &Skeleton::operator=(const Skeleton &other) noexcept {
    if (this != &other) {
        scale = other.scale;
        movableBones = other.movableBones;
        bones = other.bones;
        // We need to reset all pointer in bones
        for (std::size_t i = 0; i < bones.size(); ++i) {
            if (bones[i].parent != nullptr) {
                bones[i].parent = &bones[other.bones[i].parent->idx];
            }
            if (bones[i].child != nullptr) {
                bones[i].child = &bones[other.bones[i].child->idx];
            }
            if (bones[i].sibling != nullptr) {
                bones[i].sibling = &bones[other.bones[i].sibling->idx];
            }
        }
        bone_graphics = other.bone_graphics;
    }
    return *this;
}

Skeleton &Skeleton::operator=(Skeleton &&other) noexcept {
    if (this != &other) {
        scale = other.scale;
        movableBones = other.movableBones;
        bones = std::move(other.bones);
        bone_graphics = std::move(other.bone_graphics);
    }
    return *this;
}

double Skeleton::getScale() const { return scale; }

int Skeleton::getBoneNum() const { return static_cast<int>(bones.size()); }

int Skeleton::getMovableBoneNum() const { return movableBones; }

Bone *Skeleton::getBonePointer(const std::string &name) {
    for (int i = 0; i < bones.size(); ++i) {
        if (name == bones[i].name) {
            return &bones[i];
        }
    }
    return nullptr;
}

Bone *Skeleton::getBonePointer(const int bone_idx) { return &bones[bone_idx]; }

void Skeleton::setBoneColor(const Eigen::Vector4f &boneColor) {
    for (int i = 0; i < bones.size(); ++i) bone_graphics[i].setTexture(boneColor);
}

void Skeleton::setModelMatrices() {
    for (int i = 0; i < bone_graphics.size(); ++i) {
        auto &&bone = bones[i];
        Eigen::Vector4d trans = 0.5 * (bone.start_position + bone.end_position);
        Eigen::Affine3d model = bone.global_facing;
        model.prerotate(bone.rotation.rotation()).pretranslate(trans.head<3>());
        bone_graphics[i].setModelMatrix(model.cast<float>());
    }
}

void Skeleton::render(graphics::Program *program) {
    for (int i = 0; i < bone_graphics.size(); ++i) {
        bone_graphics[i].render(program);
    }
}

bool Skeleton::readASFFile(const util::fs::path &file_name) {
    std::ifstream input_stream(file_name);
    if (!input_stream) {
        std::cerr << "Failed to open " << file_name << std::endl;
        return false;
    }
    // ignore header information
    std::string str, keyword;
    while (true) {
        std::getline(input_stream, str);
        if (str.compare(0, 9, ":bonedata") == 0) {
            break;
        }
    }
    // Ignore begin
    input_stream.ignore(1024, '\n');
    bool done = false;
    while (!done) {
        auto &&current_bone = bones.emplace_back();
        while (true) {
            input_stream >> keyword;
            if (keyword == "end") {
                break;
            }
            // finish read bone data, start setup bone hierarchy
            if (keyword == ":hierarchy") {
                done = true;
                bones.pop_back();
                break;
            }
            // id of bone
            if (keyword == "id") {
                input_stream >> current_bone.idx;
                continue;
            }
            // name of the bone
            if (keyword == "name") {
                input_stream >> current_bone.name;
                continue;
            }
            // this line describes the bone's direction vector in global coordinate
            // it will later be converted to local coorinate system
            if (keyword == "direction") {
                input_stream >> current_bone.dir[0] >> current_bone.dir[1] >> current_bone.dir[2];
                continue;
            }
            // length of the bone
            if (keyword == "length") {
                input_stream >> current_bone.length;
                current_bone.length *= scale;
                continue;
            }
            // this line describes the orientation of bone's local coordinate
            // system relative to the world coordinate system
            if (keyword == "axis") {
                input_stream >> current_bone.axis[0] >> current_bone.axis[1] >> current_bone.axis[2];
                continue;
            }
            // this line describes the bone's dof
            if (keyword == "dof") {
                ++movableBones;
                std::string token;
                current_bone.dof = 0;
                std::getline(input_stream, str);
                std::istringstream iss(str);
                while (iss >> token) {
                    if (token.compare(0, 2, "rx") == 0) {
                        current_bone.dofrx = true;
                        ++current_bone.dof;
                    } else if (token.compare(0, 2, "ry") == 0) {
                        current_bone.dofry = true;
                        ++current_bone.dof;
                    } else if (token.compare(0, 2, "rz") == 0) {
                        current_bone.dofrz = true;
                        ++current_bone.dof;
                    } else if (token.compare(0, 2, "tx") == 0) {
                        current_bone.doftx = true;
                        ++current_bone.dof;
                    } else if (token.compare(0, 2, "ty") == 0) {
                        current_bone.dofty = true;
                        ++current_bone.dof;
                    } else if (token.compare(0, 2, "tz") == 0) {
                        current_bone.doftz = true;
                        ++current_bone.dof;
                    } else {
                        std::cerr << "Unknown token: " << token << std::endl;
                    }
                }
            }
        }
    }
    // skip "begin" line
    input_stream.ignore(1024, '\n');
    input_stream.ignore(1024, '\n');
    // Assign parent/child relationship to the bones
    while (true) {
        // read next line
        std::getline(input_stream, str);
        std::istringstream iss(str);
        iss >> keyword;
        // check if we are done
        if (keyword == "end") break;
        // parse this line, it contains parent followed by children
        int parent = this->getBonePointer(keyword)->idx;
        while (iss >> keyword) {
            this->setChildrenSibling(parent, getBonePointer(keyword));
        }
    }
    std::cout << bones.size() << " bones in " << file_name.string() << " are read" << std::endl;
    input_stream.close();
    return true;
}

Bone *Skeleton::getBone(Bone *ptr, const int bIndex) {
    static Bone *theptr;
    if (nullptr == ptr) {
        return nullptr;
    } else if (ptr->idx == bIndex) {
        theptr = ptr;
        return theptr;
    } else {
        this->getBone(ptr->child, bIndex);
        this->getBone(ptr->sibling, bIndex);
        return theptr;
    }
}

int Skeleton::setChildrenSibling(const int parent, Bone *pChild) {
    // Get pointer to root bone
    Bone *pParent = this->getBone(&bones[0], parent);

    if (pParent == nullptr) {
        printf("inbord bone is undefined\n");
        return 0;
    } else {
        pChild->parent = pParent;
        // if pParent bone does not have a child
        // set pChild as parent bone child
        if (pParent->child == nullptr) {
            pParent->child = pChild;
        } else {
            // if pParent bone already has a child
            // set pChils as pParent bone's child sibling
            pParent = pParent->child;
            while (pParent->sibling != nullptr) {
                pParent = pParent->sibling;
            }
            pParent->sibling = pChild;
        }
        return 1;
    }
}

/**
 * @brief Transform the direction vector (dir),
 * which is defined in character's global coordinate system in the ASF file,
 * to local coordinate
 */
void Skeleton::rotateBone2LocalCoord() {
    for (int i = 1; i < this->getBoneNum(); ++i) {
        // Transform dir vector into local coordinate system
        bones[i].dir = Eigen::Affine3d(util::rotateDegreeXYZ(-bones[i].axis)) * bones[i].dir;
    }
}

void Skeleton::computeRotationParent2Child(Bone *parent, Bone *child) {
    if (child != nullptr) {
        Eigen::Quaterniond child2parent = util::rotateDegreeXYZ(-parent->axis) * util::rotateDegreeZYX(child->axis);
        child2parent.normalize();
        child->rot_parent_current = child2parent.toRotationMatrix();
    }
}

void Skeleton::computeRotation2ParentCoord(Bone *bone) {
    Eigen::Quaterniond rootRotation = util::rotateDegreeZYX(bone[0].axis);
    rootRotation.normalize();
    bone[0].rot_parent_current = rootRotation.toRotationMatrix();
    // Compute rot_parent_current for all other bones
    for (int i = 0; i < this->getBoneNum(); i++) {
        if (bone[i].child != nullptr) {
            this->computeRotationParent2Child(&bone[i], bone[i].child);
            // compute parent child siblings...
            Bone *tmp = nullptr;
            if (bone[i].child != nullptr) {
                tmp = (bone[i].child)->sibling;
            }
            while (tmp != nullptr) {
                this->computeRotationParent2Child(&bone[i], tmp);
                tmp = tmp->sibling;
            }
        }
    }
}

void Skeleton::setBoneGraphics() {
    bone_graphics.resize(bones.size());
    for (int i = 0; i < bones.size(); ++i) {
        bone_graphics[i].setTexture(Eigen::Vector4f(0.6f, 0.6f, 0.0f, 1.0f));
        auto &&bone = bones[i];
        Eigen::Vector4d rotaion_axis = Eigen::Vector4d::UnitZ().cross3(bone.dir);
        double dot_val = Eigen::Vector4d::UnitZ().dot(bone.dir);
        double cross_val = rotaion_axis.norm();
        rotaion_axis.normalize();
        double theta = atan2(cross_val, dot_val);
        bone.global_facing = Eigen::AngleAxisd(theta, rotaion_axis.head<3>());
        bone.global_facing.scale(Eigen::Vector3d(1.0, 1.0, bone.length));
    }
}

}  // namespace acclaim

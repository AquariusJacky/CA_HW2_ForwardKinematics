#include "simulation/kinematics.h"

#include "Eigen/Dense"

#include "acclaim/bone.h"
#include "util/helper.h"

#define M_PI 3.1415

namespace kinematics {
void forwardSolver(const acclaim::Posture& posture, acclaim::Bone* bone) {
    // TODO
    // This function will be called with bone == root bone of the skeleton
    // You should set these variables:
    //     bone->start_position = Eigen::Vector4d::Zero();
    //     bone->end_position = Eigen::Vector4d::Zero();
    //     bone->rotation = Eigen::Matrix4d::Zero();
    // The sample above just set everything to zero

    if (bone == nullptr) { return; }

    int bone_idx = bone->idx;
    acclaim::Bone* parentBone = bone->parent;

    bone->start_position = posture.bone_translations[bone_idx];
    bone->rotation =
        bone->rot_parent_current * Eigen::Affine3d(util::rotateDegreeZYX(posture.bone_rotations[bone_idx]));

    if (parentBone != nullptr) {
        bone->start_position = parentBone->end_position + posture.bone_translations[bone_idx];
        bone->rotation = parentBone->rotation * bone->rotation;
    }

    bone->end_position = bone->start_position + bone->rotation * (bone->dir * bone->length);

    forwardSolver(posture, bone->sibling);
    forwardSolver(posture, bone->child);

    return;
}

std::vector<acclaim::Posture> timeWarper(const std::vector<acclaim::Posture>& postures, int keyframe_old,
                                         int keyframe_new) {
    int total_frames = static_cast<int>(postures.size());
    int total_bones = static_cast<int>(postures[0].bone_rotations.size());

    double ratio = double(keyframe_old) / double(keyframe_new);
    int difference = keyframe_new - keyframe_old;

    std::vector<acclaim::Posture> new_postures = postures;
    for (int i = 0; i < total_frames; ++i) {

        int newKeyframe = i;
        double oldKeyframeNum = ratio * i;
        if (i > keyframe_new) {
            oldKeyframeNum = i - difference;
        }

        if (oldKeyframeNum >= total_frames) {
            break;
        }

        int lowerBound = oldKeyframeNum;
        int upperBound = lowerBound + 1;
        double ratioBetweenBoundary = oldKeyframeNum - double(lowerBound);

        for (int j = 0; j < total_bones; ++j) {
            // TODO
            // You should set these variables:
            //     new_postures[i].bone_translations[j] = postures[i].bone_translations[j];
            //     new_postures[i].bone_rotations[j] = postures[i].bone_rotations[j];
            // The sample above just change nothing

            if (ratioBetweenBoundary == 0) {
                new_postures[i].bone_translations[j] = postures[oldKeyframeNum].bone_translations[j];
                new_postures[i].bone_rotations[j] = postures[oldKeyframeNum].bone_rotations[j];
                continue;
            }

            Eigen::Vector4d translationDifference =
                postures[upperBound].bone_translations[j] - postures[lowerBound].bone_translations[j];
            new_postures[i].bone_translations[j] =
                postures[lowerBound].bone_translations[j] + translationDifference * ratioBetweenBoundary;

            Eigen::Quaterniond q1;
            q1 = Eigen::AngleAxisd(postures[lowerBound].bone_rotations[j][0] * M_PI / 180, Eigen::Vector3d::UnitX()) *
                 Eigen::AngleAxisd(postures[lowerBound].bone_rotations[j][1] * M_PI / 180, Eigen::Vector3d::UnitY()) *
                 Eigen::AngleAxisd(postures[lowerBound].bone_rotations[j][2] * M_PI / 180, Eigen::Vector3d::UnitZ());

            Eigen::Quaterniond q2;
            q2 = Eigen::AngleAxisd(postures[upperBound].bone_rotations[j][0] * M_PI / 180, Eigen::Vector3d::UnitX()) *
                 Eigen::AngleAxisd(postures[upperBound].bone_rotations[j][1] * M_PI / 180, Eigen::Vector3d::UnitY()) *
                 Eigen::AngleAxisd(postures[upperBound].bone_rotations[j][2] * M_PI / 180, Eigen::Vector3d::UnitZ());

            Eigen::Vector3d eulerAngles = q1.slerp(ratioBetweenBoundary, q2).normalized().toRotationMatrix().eulerAngles(0, 1, 2);
            new_postures[i].bone_rotations[j] = Eigen::Vector4d(
                eulerAngles[0] * 180 / M_PI, eulerAngles[1] * 180 / M_PI, eulerAngles[2] * 180 / M_PI, 0);
        }
    }
    return new_postures;
}
}  // namespace kinematics

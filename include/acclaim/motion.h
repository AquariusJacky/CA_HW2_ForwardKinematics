#pragma once

#include <memory>
#include <vector>

#include "Eigen/Core"

#include "posture.h"
#include "skeleton.h"
#include "util/filesystem.h"

namespace acclaim {

class Motion final {
 public:
    Motion(const util::fs::path &amc_file, std::unique_ptr<Skeleton> &&skeleton) noexcept;
    Motion() noexcept;
    Motion(const Motion &) noexcept;
    Motion(Motion &&) noexcept;

    Motion &operator=(const Motion &) noexcept;
    Motion &operator=(Motion &&) noexcept;
    // You need this for alignment otherwise it may crash
    // Ref: https://eigen.tuxfamily.org/dox/group__TopicStructHavingEigenMembers.html
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    // get the underlying skeleton
    const std::unique_ptr<Skeleton> &getSkeleton() const;
    // get total frame of the motion
    int getFrameNum() const;
    // Forward kinematics
    void setBoneTransform(int frame_idx);
    // Time warpping
    void timeWarper(int oldframe, int newframe);
    // read motion data from file
    bool readAMCFile(const util::fs::path &file_name);

 private:
    std::unique_ptr<Skeleton> skeleton;
    std::vector<Posture> postures;
};
}  // namespace acclaim

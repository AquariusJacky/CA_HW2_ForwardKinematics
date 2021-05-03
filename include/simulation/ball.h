#pragma once

#include <memory>

#include "Eigen/Core"
#include "graphics/sphere.h"

namespace acclaim {
struct Bone;
}
namespace kinematics {
class Ball {
 public:
    explicit Ball(const acclaim::Bone* bone) noexcept;
    // no copy constructor
    Ball(const Ball&) = delete;
    Ball(Ball&&) noexcept;
    // You need this for alignment otherwise it may crash
    // Ref: https://eigen.tuxfamily.org/dox/group__TopicStructHavingEigenMembers.html
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Ball& operator=(const Ball&) = delete;
    Ball& operator=(Ball&&) noexcept;
    // Get the underlying graphics object
    std::unique_ptr<graphics::Sphere>& getGraphics();
    // Calculate and set position at specific time stamp
    void set_model_matrix(int time);
    // Reset the ball
    void release();

 private:
    Eigen::Vector4d start_pos = Eigen::Vector4d(-0.7362313318, 15.969798215, 3.448259997, 0.0);
    Eigen::Vector4d gravity = Eigen::Vector4d(0.0, -0.098, 0.0, 0.0);
    bool catched = false;
    const acclaim::Bone* catcher;
    std::unique_ptr<graphics::Sphere> graphics;
};
}  // namespace kinematics

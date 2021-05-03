#include "simulation/ball.h"
#include <utility>
#include "acclaim/bone.h"
namespace kinematics {
Ball::Ball(const acclaim::Bone* bone) noexcept : catcher(bone), graphics(std::make_unique<graphics::Sphere>()) {}
Ball::Ball(Ball&& other) noexcept
    : start_pos(std::move(other.start_pos)),
      gravity(std::move(other.gravity)),
      catched(other.catched),
      catcher(other.catcher),
      graphics(std::move(other.graphics)) {}

Ball& Ball::operator=(Ball&& other) noexcept {
    if (this != &other) {
        start_pos = std::move(other.start_pos);
        gravity = std::move(other.gravity);
        catched = other.catched;
        catcher = other.catcher;
        graphics = std::move(other.graphics);
    }
    return *this;
}
void Ball::release() { catched = false; }

std::unique_ptr<graphics::Sphere>& Ball::getGraphics() { return graphics; }

void Ball::set_model_matrix(int time) {
    if (time == 0) catched = false;
    Eigen::Affine3d trans = Eigen::Affine3d::Identity();
    Eigen::Vector4d current_position = start_pos + 0.005 * time * time * gravity;
    Eigen::Vector4d center = (catcher->start_position + catcher->end_position) * 0.5;
    if (!catched && (center - current_position).norm() < 1E-6) catched = true;
    if (catched) {
        trans.translate(center.head<3>());
    } else if (current_position.y() >= 0.125) {
        trans.translate(current_position.head<3>());
    } else {
        current_position.y() = 0.125;
        trans.translate(current_position.head<3>());
    }
    trans.scale(Eigen::Vector3d(0.125, 0.125, 0.125));
    graphics->setModelMatrix(trans.cast<float>());
}
}  // namespace kinematics

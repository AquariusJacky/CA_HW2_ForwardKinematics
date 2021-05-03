#pragma once
// ----------------------------------------------------------------
// You MUST include this file if you use any Fixed-size vectorizable Eigen objects in vectors
// Ref: https://eigen.tuxfamily.org/dox/group__TopicFixedSizeVectorizable.html
// Otherwise, the program will probably crash due to unaligned SIMD operations
// ----------------------------------------------------------------
#include "Eigen/Core"
#include "Eigen/Geometry"
#include "Eigen/StdVector"
// Defines special std::vectors
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector4f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Vector4d)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Affine3f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Affine3d)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix4f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix4d)

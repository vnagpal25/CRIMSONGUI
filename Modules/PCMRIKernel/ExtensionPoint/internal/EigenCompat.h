#pragma once

#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#ifndef EIGEN_NO_DEPRECATED_WARNING
#define EIGEN_NO_DEPRECATED_WARNING
#endif

#ifndef EIGEN_NOEXCEPT
#define EIGEN_NOEXCEPT noexcept
#endif

#ifndef EIGEN_NOEXCEPT_IF
#define EIGEN_NOEXCEPT_IF(x) noexcept(x)
#endif

#include <Eigen/Core>

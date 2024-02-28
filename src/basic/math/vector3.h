//
// Created by xmyci on 20/02/2024.
//

#ifndef TETGEO_VECTOR3_H
#define TETGEO_VECTOR3_H

#include <cassert>
#include <algorithm>
#include <cmath>
#include <limits>

namespace base_type {

    class Vector3 {

    public:
        double x{0.f};
        double y{0.f};
        double z{0.f};

    public:
        Vector3() = default;

        Vector3(double x_, double y_, double z_) : x{x_}, y{y_}, z{z_} {}

        explicit Vector3(const double coords[3]) : x{coords[0]}, y{coords[1]}, z{coords[2]} {}

        double operator[](size_t i) const {
            return *(&x + i);
        }

        double &operator[](size_t i) {
            return *(&x + i);
        }

        /// Pointer accessor for direct copying
        double *ptr() { return &x; }

        /// Pointer accessor for direct copying
        const double *ptr() const { return &x; }

        bool operator==(const Vector3 &rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); }

        bool operator!=(const Vector3 &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

        // arithmetic operations
        Vector3 operator+(const Vector3 &rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }

        Vector3 operator-(const Vector3 &rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }

        Vector3 operator*(double scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }

        Vector3 operator*(const Vector3 &rhs) const { return Vector3(x * rhs.x, y * rhs.y, z * rhs.z); }

        Vector3 operator/(double scalar) const {
            assert(scalar != 0.0);
            return Vector3(x / scalar, y / scalar, z / scalar);
        }

        Vector3 operator/(const Vector3 &rhs) const {
            assert((rhs.x != 0 && rhs.y != 0 && rhs.z != 0));
            return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
        }

        const Vector3 &operator+() const { return *this; }

        Vector3 operator-() const { return Vector3(-x, -y, -z); }

        // overloaded operators to help Vector3
        friend Vector3 operator*(double scalar, const Vector3 &rhs) {
            return Vector3(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z);
        }

        friend Vector3 operator/(double scalar, const Vector3 &rhs) {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            return Vector3(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z);
        }

        friend Vector3 operator+(const Vector3 &lhs, double rhs) {
            return Vector3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
        }

        friend Vector3 operator+(double lhs, const Vector3 &rhs) {
            return Vector3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
        }

        friend Vector3 operator-(const Vector3 &lhs, double rhs) {
            return Vector3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
        }

        friend Vector3 operator-(double lhs, const Vector3 &rhs) {
            return Vector3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
        }

        // arithmetic updates
        Vector3 &operator+=(const Vector3 &rhs) {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        Vector3 &operator+=(double scalar) {
            x += scalar;
            y += scalar;
            z += scalar;
            return *this;
        }

        Vector3 &operator-=(const Vector3 &rhs) {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        Vector3 &operator-=(double scalar) {
            x -= scalar;
            y -= scalar;
            z -= scalar;
            return *this;
        }

        Vector3 &operator*=(double scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        Vector3 &operator*=(const Vector3 &rhs) {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        Vector3 &operator/=(double scalar) {
            assert(scalar != 0.0);
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        Vector3 &operator/=(const Vector3 &rhs) {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        /** Returns the length (magnitude) of the vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        length (e.g. for just comparing lengths) use squaredLength()
        instead.
        */

        double length() const { return std::hypot(x, y, z); }

        /** Returns the square of the length(magnitude) of the vector.
        @remarks
        This  method is for efficiency - calculating the actual
        length of a vector requires a square root, which is expensive
        in terms of the operations required. This method returns the
        square of the length of the vector, i.e. the same as the
        length but before the square root is taken. Use this if you
        want to find the longest / shortest vector without incurring
        the square root.
        */
        double squaredLength() const { return x * x + y * y + z * z; }

        /** Returns the distance to another vector.
        @warning
        This operation requires a square root and is expensive in
        terms of CPU operations. If you don't need to know the exact
        distance (e.g. for just comparing distances) use squaredDistance()
        instead.
        */

        double distance(const Vector3 &rhs) const { return (*this - rhs).length(); }

        /** Returns the square of the distance to another vector.
        @remarks
        This method is for efficiency - calculating the actual
        distance to another vector requires a square root, which is
        expensive in terms of the operations required. This method
        returns the square of the distance to another vector, i.e.
        the same as the distance but before the square root is taken.
        Use this if you want to find the longest / shortest distance
        without incurring the square root.
        */

        double squaredDistance(const Vector3 &rhs) const { return (*this - rhs).squaredLength(); }

        /** Calculates the dot (scalar) product of this vector with another.
        @remarks
        The dot product can be used to calculate the angle between 2
        vectors. If both are unit vectors, the dot product is the
        cosine of the angle; otherwise the dot product must be
        divided by the product of the lengths of both vectors to get
        the cosine of the angle. This result can further be used to
        calculate the distance of a point from a plane.
        @param
        vec Vector with which to calculate the dot product (together
        with this one).
        @returns
        A double representing the dot product value.
        */

        double dot(const Vector3 &vec) const { return x * vec.x + y * vec.y + z * vec.z; }

        /** Normalizes the vector.
        @remarks
        This method normalizes the vector such that it's
        length / magnitude is 1. The result is called a unit vector.
        @note
        This function will not crash for zero-sized vectors, but there
        will be no changes made to their components.
        @returns The previous length of the vector.
        */

        Vector3 normalise() {
            double length = std::hypot(x, y, z);
            if (length == 0.f) {
                assert(false);
                return Vector3(x, y, z);
            }

            return Vector3(x / length, y / length, z / length);
        }

        Vector3 cross(const Vector3 &rhs) const {
            return Vector3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
        }


    };
}
#endif //TETGEO_VECTOR3_H

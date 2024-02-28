//
// Created by xmyci on 20/02/2024.
//

#ifndef TETGEO_PREDICATES_WRAPPER_H
#define TETGEO_PREDICATES_WRAPPER_H

#include "basic/math/vector3.h"


namespace Geometrical_Predicates {
    using namespace base_type;

    struct IntersectionResult3d {
        bool intersect;
        Vector3 intersectionPoint;
    };

    struct Triangle3d {
        Vector3 p1;
        Vector3 p2;
        Vector3 p3;
    };

    struct Edge {
        Vector3 p1;
        Vector3 p2;
    };

    struct Line {
        Vector3 p1;
        Vector3 p2;
    };

    struct Line3d {
        Vector3 p1;
        Vector3 p2;
    };

    struct Ray3d {
        Vector3 from;
        Vector3 to;
    };

    struct Plane {
        Vector3 p;
        Vector3 normal;
    };

    inline Vector3 cross(const Vector3 &p1, const Vector3 &p2);

    inline double dot(const Vector3 &p1, const Vector3 &p2);

    inline bool toleft(const Vector3 &p1, const Vector3 &p2, const Vector3 &s);

    inline bool toleft(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &s);

    inline bool colinear(const Vector3 &a, const Vector3 &b, const Vector3 &c);

    inline bool coplanar(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &d);

    inline double plane_point_distance(Plane plane, Vector3 point);

    inline double vector_length_sqr(Vector3 a);

    inline double vector_length(Vector3 a);

    inline double vector_length_sqr(Vector3 a, Vector3 b);

    inline double vector_length(Vector3 a, Vector3 b);

    //implement begin
    inline Vector3 cross(const Vector3 &p1, const Vector3 &p2) {
        //(u1, u2, u3) x(v1, v2, v3) = (u2v3 - u3v2; u3v1 - u1v3, u1v2 - u2v1)
        double s1, s2, s3;
        s1 = p1.y * p2.z - p1.z * p2.y;
        s2 = p1.z * p2.x - p1.x * p2.z;
        s3 = p1.x * p2.y - p1.y * p2.x;
        return Vector3(s1, s2, s3);
    }

    inline double dot(const Vector3 &p1, const Vector3 &p2) {
        return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
    }

    inline bool toleft(const Vector3 &p1, const Vector3 &p2, const Vector3 &s) {
        double value = p1.x * p2.y - p1.y * p2.x
                       + p2.x * s.y - p2.y * s.x
                       + s.x * p1.y - s.y * p1.x;
        return value > 0;
    }

    inline bool toleft(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &s) {
        //ORIENT3D

        double a11, a12, a13;
        double a21, a22, a23;
        double a31, a32, a33;

        a11 = p1.x - s.x;
        a12 = p1.y - s.y;
        a13 = p1.z - s.z;
        a21 = p2.x - s.x;
        a22 = p2.y - s.y;
        a23 = p2.z - s.z;
        a31 = p3.x - s.x;
        a32 = p3.y - s.y;
        a33 = p3.z - s.z;

        double value = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a31 * a22 * a13 - a32 * a23 * a11 - a21 * a12 * a33;

        return value > 1e-8;
    }

    inline bool colinear(const Vector3 &a, const Vector3 &b, const Vector3 &c) {
        return ((c.z - a.z) * (b.y - a.y) -
                (b.z - a.z) * (c.y - a.y)) == 0 && \
         ((b.z - a.z) * (c.x - a.x) -
          (b.x - a.x) * (c.z - a.z)) == 0 && \
         ((b.x - a.x) * (c.y - a.y) -
          (b.y - a.y) * (c.x - a.x)) == 0;
    }

    inline bool coplanar(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &d) {
        assert(colinear(a, b, c) == false);
        double disance = plane_point_distance({a, (b - a).cross(c - a).normalise()}, d);
        return disance < 1e-4;
    }

    inline double plane_point_distance(Plane plane, Vector3 point) {
        Vector3 Q = point - plane.p;
        return std::fabs(dot(/*VectorNormal*/(plane.normal), Q));
    }

    inline double vector_length_sqr(Vector3 a) {
        return (a.x) * (a.x) + (a.y) * (a.y) + (a.z) * (a.z);
    }

    inline double vector_length(Vector3 a) {
        return std::sqrt((a.x) * (a.x) + (a.y) * (a.y) + (a.z) * (a.z));
    }

    inline double vector_length_sqr(Vector3 a, Vector3 b) {
        return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z);
    }

    inline double vector_length(Vector3 a, Vector3 b) {
        return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
    }
}


#endif //TETGEO_PREDICATES_WRAPPER_H

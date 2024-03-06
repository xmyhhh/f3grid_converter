#pragma once

#include <vector>

#include "basic/typedef.h"
#include "basic/geometrical predicates/predicates_wrapper.h"
#include "mesh loader/mesh_loader.h"
#include "basic/data structure/memory_pool.h"
#include "basic/math/vector3.h"
#include "config/config_loader.h"

using namespace base_type;

inline base_type::Vector3 VectorNormal(base_type::Vector3 a) {
    using namespace Geometrical_Predicates;

    return a / std::sqrt(vector_length_sqr(a));
}

inline base_type::Vector3 Rotate3d(base_type::Vector3 point, double yaw, double pitch, double roll) {
    using namespace Geometrical_Predicates;

    //https://en.wikipedia.org/wiki/Rotation_matrix
    //yaw * pitch * roll * point
    //A yaw is a counterclockwise rotation of $ \alpha$ about the $ z$-axis.
    //A pitch is a counterclockwise rotation of $ \beta$ about the $ y$-axis.
    //A roll is a counterclockwise rotation of $ \gamma$ about the $ x$-axis.

    double a = yaw / 180;
    double b = pitch / 180;
    double r = roll / 180;
    using namespace std;

    double cos_a = cos(a), cos_b = cos(b), cos_r = cos(r);
    double sin_a = sin(a), sin_b = sin(b), sin_r = sin(r);

    double x = point.x * (cos_a * cos_b) + point.y * (cos_a * sin_b * sin_r - sin_a * cos_r) + point.z * (cos_a * sin_b * cos_r + sin_a * sin_r);
    double y = point.x * (sin_a * cos_b) + point.y * (sin_a * sin_b * sin_r + cos_a * cos_r) + point.z * (sin_a * sin_b * cos_r - cos_a * sin_r);
    double z = point.x * (-sin_b) + point.y * (cos_b * sin_r) + point.z * (cos_b * cos_r);

    return base_type::Vector3(x, y, z);
}


char *read_line(char *string, FILE *infile, int *linenumber) {
    char *result;

    // Search for a non-empty line.
    do {
        result = fgets(string, 2048, infile);
        if (linenumber) (*linenumber)++;
        if (result == (char *) NULL) {
            return (char *) NULL;
        }
        // Skip white spaces.
        while ((*result == ' ') || (*result == '\t')) result++;
        // If it's end of line, read another line and try again.
    } while ((*result == '\0') || (*result == '\r') || (*result == '\n'));
    return result;
}

bool isPointInsideTriangle(base_type::Triangle3d tri, base_type::Vector3 p) {
    using namespace Geometrical_Predicates;

    if (tri.p1 == p || tri.p2 == p || tri.p3 == p)
        return true;

    auto get_triangle_S = [](base_type::Triangle3d t) -> double {
        auto A = t.p1;
        auto B = t.p2;
        auto C = t.p3;

        double S = vector_length(cross(B - A, C - A));
        return S;
    };

    double S_ABC, S_ABP, S_ACP, S_BCP;
    S_ABC = get_triangle_S(tri);
    S_ABP = get_triangle_S({tri.p1, tri.p2, p});
    S_ACP = get_triangle_S({tri.p1, tri.p3, p});
    S_BCP = get_triangle_S({tri.p2, tri.p3, p});

    return fabs(S_ABC - (S_ABP + S_ACP + S_BCP)) < 1e-8;


}

Geometrical_Predicates::IntersectionResult3d RayIntersectionCalulate_Triangle(base_type::Triangle3d tri, base_type::Ray3d ray, bool include_border) {
    using namespace Geometrical_Predicates;

    auto ray_dir = (ray.to - ray.from);
    //auto ray_dir_normal = VectorNormal(ray_dir);
    auto ray_origin = ray.from;

    auto u = tri.p2 - tri.p1;
    auto v = tri.p3 - tri.p1;
    auto Q = tri.p1;

    base_type::Vector3 cvu = cross(u, v);
    base_type::Vector3 plane_normal = VectorNormal(cvu);

    auto denom = dot(plane_normal, ray_dir);
    if (fabs(denom) < 1e-8) {
        return {false};
    }

    double D = dot(plane_normal, Q);
    double t = (D - dot(plane_normal, ray_origin)) / denom;
    if (t > 1 || t < 0) {
        return {false};
    }

    base_type::Vector3 intersection = ray_origin + ray_dir * t;

    if (include_border) {
        bool b = isPointInsideTriangle(tri, intersection);//include border and non-border

        if (b)
            return {true, intersection};
    }
    else {  //include non-border
        base_type::Vector3 planar_hitpt_vector = intersection - Q;
        auto w = cvu / dot(cvu, cvu);
        auto alpha = dot(w, cross(planar_hitpt_vector, v));
        auto beta = dot(w, cross(u, planar_hitpt_vector));

        //auto aa = alpha * u + beta * v + Q;
        //ASSERT(VectorLengthSqr(aa - intersection) < 1e-8);

        if (fabs(alpha) < 1e-8 || fabs(beta) < 1e-8) {
            return {false};
        }

        if ((alpha + beta) < 0.99999999999 && alpha > 1e-8 && beta > 1e-8) {
            return {true, intersection};
        }
    }
    return {false};
}


struct PhysicalGroup_2D {
    std::vector<Face *> face_array;
    std::vector<Face *> boundary_face_array;


    std::vector<Vertex *> get_vtx() {
        std::vector<Vertex *> vtx_array;
        for (const auto &f: face_array) {
            if (std::find(vtx_array.begin(), vtx_array.end(), f->p1) == vtx_array.end()) {
                vtx_array.push_back(f->p1);
            }
            if (std::find(vtx_array.begin(), vtx_array.end(), f->p2) == vtx_array.end()) {
                vtx_array.push_back(f->p2);
            }
            if (std::find(vtx_array.begin(), vtx_array.end(), f->p3) == vtx_array.end()) {
                vtx_array.push_back(f->p3);
            }
        }
        return vtx_array;
    }
};

struct Unwrap {
    MemoryPool vertex_pool;
    MemoryPool edge_pool;
    MemoryPool face_pool;
    MemoryPool tetrahedra_pool;

    int size = 40;
    std::vector<PhysicalGroup_2D> phy_group_array;

    Unwrap() {
        vertex_pool.initializePool(sizeof(base_type::Vertex), 1000 * 1.2, 8, 32);
        tetrahedra_pool.initializePool(sizeof(base_type::Tetrahedra), 1000 * 3 * 1.2, 8, 32);
        face_pool.initializePool(sizeof(base_type::Face), 1000 * 1.2, 8, 32);
        edge_pool.initializePool(sizeof(base_type::Edge), 1000 * 3 * 1.2, 8, 32);

    }

    bool init_from_filedata(Mesh_Loader::FileData data) {


        for (int i = 0; i < data.numberOfPoints; i++) {
            base_type::Vertex::allocate_from_pool(&vertex_pool, {data.pointList[i * 3], data.pointList[i * 3 + 1], data.pointList[i * 3 + 2]});
        }
        for (int i = 0; i < data.numberOfCell; i++) {
            auto &cell = data.cellList[i];
            assert(cell.numberOfPoints == 4);
            base_type::Vertex *p1 = (base_type::Vertex *) vertex_pool[cell.pointList[0]];
            base_type::Vertex *p2 = (base_type::Vertex *) vertex_pool[cell.pointList[1]];
            base_type::Vertex *p3 = (base_type::Vertex *) vertex_pool[cell.pointList[2]];
            base_type::Vertex *p4 = (base_type::Vertex *) vertex_pool[cell.pointList[3]];
            auto t = base_type::Tetrahedra::allocate_from_pool(&tetrahedra_pool, p1, p2, p3, p4);
            if (config.array_to_number) {
                if (config.export_materialids_using_slot < data.cellDataInt.size()) {
                    auto iter = data.cellDataInt.begin();
                    std::advance(iter, config.export_materialids_using_slot);
                    t->type_id = iter->second.content[i];
                }
                else
                    t->type_id = data.cellDataInt.begin()->second.content[i];
            }

        }
        update_tet_neightbors();
        create_face_and_edge();
        boundary_face_mark();
        return true;
    }

    bool save_file(std::string path_base) {
        using namespace Mesh_Loader;
        {
            FileData data;

            data.numberOfPoints = vertex_pool.size();
            data.pointList = new double[data.numberOfPoints * 3];

            for (int j = 0; j < vertex_pool.size(); j++) {
                const auto &vtx = (Vertex *) vertex_pool[j];
                data.pointList[j * 3] = vtx->position.x;
                data.pointList[j * 3 + 1] = vtx->position.y;
                data.pointList[j * 3 + 2] = vtx->position.z;

            }
            if (config.array_to_number)
                data.cellDataInt["MaterialIDs"];
            data.numberOfCell = tetrahedra_pool.size();
            data.cellList = new Cell[data.numberOfCell];
            for (int j = 0; j < tetrahedra_pool.size(); j++) {
                const auto &t = (Tetrahedra *) tetrahedra_pool[j];
                data.cellList[j].pointList = new int[4];
                data.cellList[j].numberOfPoints = 4;
                if (config.array_to_number)
                    data.cellDataInt["MaterialIDs"].content.push_back(t->type_id);
                data.cellList[j].pointList[0] = t->p1->static_index;
                data.cellList[j].pointList[1] = t->p2->static_index;
                data.cellList[j].pointList[2] = t->p3->static_index;
                data.cellList[j].pointList[3] = t->p4->static_index;

            }
            save_vtu((path_base + "/" + "domain" + ".vtu").c_str(), data);
        }


        for (int i = 0; i < phy_group_array.size(); i++) {
            auto &phg = phy_group_array[i];
            auto phg_vtx_array = phg.get_vtx();
            FileData data;

            data.numberOfPoints = phg_vtx_array.size();
            data.pointList = new double[data.numberOfPoints * 3];

            data.pointDataUInt64["bulk_node_ids"];
            data.pointDataUInt64["bulk_element_ids"];

            for (int j = 0; j < phg_vtx_array.size(); j++) {
                const auto &vtx = phg_vtx_array[j];
                data.pointList[j * 3] = vtx->position.x;
                data.pointList[j * 3 + 1] = vtx->position.y;
                data.pointList[j * 3 + 2] = vtx->position.z;
                data.pointDataUInt64["bulk_node_ids"].content.push_back(vtx->static_index);
            }

            data.numberOfCell = phg.face_array.size();
            data.cellList = new Cell[data.numberOfCell];

            for (int j = 0; j < phg.face_array.size(); j++) {
                const auto &face = phg.face_array[j];
                data.cellList[j].pointList = new int[3];
                data.cellList[j].numberOfPoints = 3;

                //data.cellList[j].cellattr = new double[1];
                auto it = std::find(phg_vtx_array.begin(), phg_vtx_array.end(), face->p1);
                assert(it != phg_vtx_array.end());

                data.cellList[j].pointList[0] = std::distance(phg_vtx_array.begin(), it);
                it = std::find(phg_vtx_array.begin(), phg_vtx_array.end(), face->p2);
                assert(it != phg_vtx_array.end());
                data.cellList[j].pointList[1] = std::distance(phg_vtx_array.begin(), it);
                it = std::find(phg_vtx_array.begin(), phg_vtx_array.end(), face->p3);
                assert(it != phg_vtx_array.end());
                data.cellList[j].pointList[2] = std::distance(phg_vtx_array.begin(), it);

                data.pointDataUInt64["bulk_element_ids"].content.push_back(face->disjoin_tet[0] != nullptr ? face->disjoin_tet[0]->static_index : face->disjoin_tet[1]->static_index);
            }
            save_vtu((path_base + "/" + std::to_string(i) + ".vtu").c_str(), data);
            //data.free_self();
        }
        return true;
    }

    void update_tet_neightbors() {

        for (int i = 0; i < tetrahedra_pool.size(); i++) {
            for (int j = 0; j < 4; j++) {
                auto t1 = (Tetrahedra *) tetrahedra_pool[i];
                t1->neighbors[j] = nullptr;
            }
        }

        for (int i = 0; i < tetrahedra_pool.size(); i++) {
            auto t1 = (Tetrahedra *) tetrahedra_pool[i];

            for (int j = 0; j < 4; j++) {
                auto vtx = t1->get_vtx(j);
                for (auto t2: *vtx->connect_tetrahedra_array) {
                    if (t1 == t2)
                        continue;

                    if (Tetrahedra::is_tetrahedra_disjoin(t1, t2)) {
                        //mark t1 t2 as neighbor
                        int k = Tetrahedra::find_disjoin_tet_non_share_vtx_index_in_t2(t1, t2);
                        t2->neighbors[k] = t1;

                        k = Tetrahedra::find_disjoin_tet_non_share_vtx_index_in_t2(t2, t1);
                        t1->neighbors[k] = t2;
                    }
                }
            }


        }
    }

    void create_face_and_edge() {


        for (int i = 0; i < tetrahedra_pool.size(); i++) {

            auto create_tet_face = [this](int tet_face_index, Tetrahedra *t) {
                if (t->faces[tet_face_index] == nullptr) {
                    if (t->neighbors[tet_face_index] != nullptr) {
                        //find adjface_vtx_index
                        auto face_index = Tetrahedra::find_disjoin_tet_non_share_vtx_index_in_t2(t, t->neighbors[tet_face_index]);
                        auto face_in_neighbor = t->neighbors[tet_face_index]->faces[face_index];
                        if (face_in_neighbor != nullptr) {
                            Tetrahedra::bind_tet_and_face(t, face_in_neighbor);
                            return;
                        }
                    }

                    auto f = Face::allocate_from_pool(&face_pool);
                    if (tet_face_index == 0) {
                        f->p1 = t->p2;
                        f->p2 = t->p3;
                        f->p3 = t->p4;
                    }
                    else if (tet_face_index == 1) {
                        f->p1 = t->p1;
                        f->p2 = t->p3;
                        f->p3 = t->p4;
                    }
                    else if (tet_face_index == 2) {
                        f->p1 = t->p1;
                        f->p2 = t->p2;
                        f->p3 = t->p4;
                    }
                    else if (tet_face_index == 3) {
                        f->p1 = t->p1;
                        f->p2 = t->p2;
                        f->p3 = t->p3;
                    }
                    else {
                        assert(false);
                    }
                    Tetrahedra::bind_tet_and_face(t, f);
                }
            };

            auto t = (Tetrahedra *) tetrahedra_pool[i];

            //create face
            create_tet_face(0, t);
            create_tet_face(1, t);
            create_tet_face(2, t);
            create_tet_face(3, t);

            for (int j = 0; j < 4; j++) {
                auto f = t->faces[j];

                auto p1 = (Vertex *) f->p1;
                auto p2 = (Vertex *) f->p2;
                auto p3 = (Vertex *) f->p3;

                auto find_edge = [](Vertex *p1, Vertex *p2, base_type::Edge *&res) -> bool {

                    for (auto e: *p1->connect_edge_array) {
                        if (e->orig == p2 || e->end == p2) {
                            res = e;
                            return true;
                        }
                    }

                    for (auto e: *p2->connect_edge_array) {
                        if (e->orig == p1 || e->end == p1) {
                            res = e;
                            return true;
                        }
                    }

                    return false;
                };

                base_type::Edge *e = nullptr;
                if (!find_edge(p1, p2, e)) {
                    e = base_type::Edge::allocate_from_pool(&edge_pool, p1, p2);
                }
                f->disjoin_edge[0] = e;
                e->connect_face_array->push_back(f);
                if (!find_edge(p2, p3, e)) {
                    e = base_type::Edge::allocate_from_pool(&edge_pool, p2, p3);
                }
                f->disjoin_edge[1] = e;
                e->connect_face_array->push_back(f);
                if (!find_edge(p3, p1, e)) {
                    e = base_type::Edge::allocate_from_pool(&edge_pool, p3, p1);
                }
                f->disjoin_edge[2] = e;
                e->connect_face_array->push_back(f);
            }

        }
    }

    void boundary_face_mark() {

        int boundary_face_num = 0;
        for (int i = 0; i < face_pool.size(); i++) {
            auto f = (Face *) face_pool[i];

            if (f->disjoin_tet[0] == nullptr || f->disjoin_tet[1] == nullptr) {
                f->is_boundary_face = true;
                boundary_face_num++;
            }
            else {
                f->is_boundary_face = false;
            }

        }


    }


};


void Unwrap_01(Unwrap &uw) {


    //Step 1: Get mesh center
    base_type::Vector3 center(0, 0, 0);
    auto uw_vtx_len = uw.vertex_pool.size();
    for (int i = 0; i < uw.vertex_pool.size(); i++) {
        auto v = (Vertex *) uw.vertex_pool[i];
        center += base_type::Vector3(v->position.x / uw_vtx_len, v->position.y / uw_vtx_len, v->position.z / uw_vtx_len);
    }

    //find six tri

    Face *six_direction_center_face[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};//x+-;y+-;z+-
    const base_type::Vector3 center_offset{0, 0, 0};
    const base_type::Vector3 center_rotation{config.r_x, config.r_y, config.r_z};
    const double len = 5000;
    base_type::Vector3 axis_x = Rotate3d({1, 0, 0}, center_rotation.x, center_rotation.y, center_rotation.z);
    base_type::Vector3 axis_y = Rotate3d({0, 1, 0}, center_rotation.x, center_rotation.y, center_rotation.z);
    base_type::Vector3 axis_z = VectorNormal(cross(axis_x, axis_y));

    for (int i = 0; i < uw.face_pool.size(); i++) {
        auto f = (Face *) uw.face_pool[i];

        if (f->is_boundary_face) {
            auto res0 = RayIntersectionCalulate_Triangle({f->p1->position, f->p2->position, f->p3->position}, {center, center + axis_x * len}, false);
            if (res0.intersect) {
                f->draw_read = true;
                six_direction_center_face[0] = f;
            }

            auto res1 = RayIntersectionCalulate_Triangle({f->p1->position, f->p2->position, f->p3->position}, {center, center - axis_x * len}, false);
            if (res1.intersect) {
                f->draw_read = true;
                six_direction_center_face[1] = f;
            }

            auto res2 = RayIntersectionCalulate_Triangle({f->p1->position, f->p2->position, f->p3->position}, {center, center + axis_y * len}, false);
            if (res2.intersect) {
                f->draw_read = true;
                six_direction_center_face[2] = f;
            }

            auto res3 = RayIntersectionCalulate_Triangle({f->p1->position, f->p2->position, f->p3->position}, {center, center - axis_y * len}, false);
            if (res3.intersect) {
                f->draw_read = true;
                six_direction_center_face[3] = f;
            }

            auto res4 = RayIntersectionCalulate_Triangle({f->p1->position, f->p2->position, f->p3->position}, {center, center + axis_z * len}, false);
            if (res4.intersect) {
                f->draw_read = true;
                six_direction_center_face[4] = f;
            }

            auto res5 = RayIntersectionCalulate_Triangle({f->p1->position, f->p2->position, f->p3->position}, {center, center - axis_z * len}, false);
            if (res5.intersect) {
                f->draw_read = true;
                six_direction_center_face[5] = f;
            }

        }
    }
    {
        assert(six_direction_center_face[0] != nullptr);
        assert(six_direction_center_face[1] != nullptr);
        assert(six_direction_center_face[2] != nullptr);
        assert(six_direction_center_face[3] != nullptr);
        assert(six_direction_center_face[4] != nullptr);
        assert(six_direction_center_face[5] != nullptr);
    }

    uw.phy_group_array.push_back({});
    uw.phy_group_array.push_back({});
    uw.phy_group_array.push_back({});
    uw.phy_group_array.push_back({});
    uw.phy_group_array.push_back({});
    uw.phy_group_array.push_back({});

    uw.phy_group_array[0].boundary_face_array.push_back(six_direction_center_face[0]);
    uw.phy_group_array[1].boundary_face_array.push_back(six_direction_center_face[1]);
    uw.phy_group_array[2].boundary_face_array.push_back(six_direction_center_face[2]);
    uw.phy_group_array[3].boundary_face_array.push_back(six_direction_center_face[3]);
    uw.phy_group_array[4].boundary_face_array.push_back(six_direction_center_face[4]);
    uw.phy_group_array[5].boundary_face_array.push_back(six_direction_center_face[5]);



    //auto grow
    auto physicalGroup_2D_grow = [](PhysicalGroup_2D &phy_group) {
        auto find_boundary_adj_face = [](Face *f, int index) -> Face * {
            for (auto connect_f: *f->disjoin_edge[index]->connect_face_array) {
                if (connect_f->is_boundary_face && connect_f != f)
                    return connect_f;
            }
            return nullptr;
        };
        auto is_face_grow_able = [](Face *f, Face *face_to_grow) {
            //assert(Face::is_disjoin_face(f, face_to_grow));
            auto n1 = Face::get_face_normal(f);
            auto n2 = Face::get_face_normal(face_to_grow);
            static const double limit = std::cos(45);

            return fabs(dot(n1, n2)) > limit;
        };

        if (phy_group.boundary_face_array.size() == 0)
            return false;

        std::vector<Face *> new_add_face_array;

        bool new_face_added = false;
        for (auto f: phy_group.boundary_face_array) {
            auto adj_f0 = find_boundary_adj_face(f, 0);
            if (adj_f0->mark == false && is_face_grow_able(phy_group.face_array.size() > 0 ? phy_group.face_array[0] : f, adj_f0)) {
                new_add_face_array.push_back(adj_f0);
                adj_f0->mark = true;
                new_face_added = true;
            }

            auto adj_f1 = find_boundary_adj_face(f, 1);
            if (adj_f1->mark == false && is_face_grow_able(phy_group.face_array.size() > 0 ? phy_group.face_array[0] : f, adj_f1)) {
                new_add_face_array.push_back(adj_f1);
                adj_f1->mark = true;
                new_face_added = true;
            }

            auto adj_f2 = find_boundary_adj_face(f, 2);
            if (adj_f2->mark == false && is_face_grow_able(phy_group.face_array.size() > 0 ? phy_group.face_array[0] : f, adj_f2)) {
                new_add_face_array.push_back(adj_f2);
                adj_f2->mark = true;
                new_face_added = true;
            }


        }

        for (auto f: phy_group.boundary_face_array) {
            phy_group.face_array.push_back(f);
        }
        phy_group.boundary_face_array.clear();

        for (auto f: new_add_face_array) {
            phy_group.boundary_face_array.push_back(f);
        }

        return new_face_added;

    };
    bool do_next = false;
    int do_times = 0;
    do {

        do_next = false;
        do_next |= physicalGroup_2D_grow(uw.phy_group_array[0]);
        do_next |= physicalGroup_2D_grow(uw.phy_group_array[1]);
        do_next |= physicalGroup_2D_grow(uw.phy_group_array[2]);
        do_next |= physicalGroup_2D_grow(uw.phy_group_array[3]);
        do_next |= physicalGroup_2D_grow(uw.phy_group_array[4]);
        do_next |= physicalGroup_2D_grow(uw.phy_group_array[5]);
        do_times++;
        //if (do_times == 10)
        //break;

    } while (do_next);

}
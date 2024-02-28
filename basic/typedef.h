#pragma once

#include <vector>
#include <set>
#include <cassert>
#include <random>
#include <iostream>
#include <string>
#include <cmath>
#include <stack>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include <Eigen/Sparse>

#include "basic/data structure/memory_pool.h"
#include "basic/math/vector3.h"
#include "basic/geometrical predicates/predicates_wrapper.h"
#include "utils/mesh loader/mesh_loader.h"
#include "utils/log/log.h"

#define  ASSERT_MSG(condition, msg) \
    if((condition) == false) { log_print(msg) ; assert(false);}

#define  WARN_MSG(condition, msg) \
    if((condition) == false) { log_print(msg) ;}


#define  _Infinity  std::numeric_limits<double>::infinity()
typedef unsigned uint;

namespace base_type {
    using namespace Geometrical_Predicates;
    struct Edge;
    struct Tetrahedra;

    struct Vertex {
        Vector3 position;
        std::vector<Tetrahedra *> *connect_tetrahedra_array;
        std::vector<Edge *> *connect_edge_array;
        uint static_index;

        static Vertex *allocate_from_pool(MemoryPool *pool) {

            auto v = (Vertex *) pool->allocate();
            v->connect_tetrahedra_array = new std::vector<Tetrahedra *>();
            v->connect_edge_array = new std::vector<Edge *>();
            v->static_index = pool->size() - 1;
            return v;
        }

        static Vertex *allocate_from_pool(MemoryPool *pool, Vector3 position) {
            auto v = (Vertex *) pool->allocate();
            v->position = position;
            v->connect_tetrahedra_array = new std::vector<Tetrahedra *>();
            v->connect_edge_array = new std::vector<Edge *>();
            v->static_index = pool->size() - 1;
            return v;
        }

    };

    struct Face {
        int face_type;
        Vertex *p1;
        Vertex *p2;
        Vertex *p3;
        Tetrahedra *disjoin_tet[2];//0 is ccw , 1 is cw
        Edge *disjoin_edge[3];
        bool is_boundary_face = false;
        bool draw_read = false;
        bool mark = false;


        static Face *allocate_from_pool(MemoryPool *pool, Vertex *_p1, Vertex *_p2, Vertex *_p3) {
            auto f = (Face *) pool->allocate();
            f->disjoin_tet[0] = nullptr;
            f->disjoin_tet[1] = nullptr;
            f->disjoin_edge[0] = nullptr;
            f->disjoin_edge[1] = nullptr;
            f->disjoin_edge[2] = nullptr;
            f->p1 = _p1;
            f->p2 = _p2;
            f->p3 = _p3;
            f->is_boundary_face = true;
            f->draw_read = false;
            f->mark = false;
            f->face_type = 0;
            return f;
        }

        static Face *allocate_from_pool(MemoryPool *pool) {
            auto f = (Face *) pool->allocate();
            f->disjoin_tet[0] = nullptr;
            f->disjoin_tet[1] = nullptr;

            f->disjoin_edge[0] = nullptr;
            f->disjoin_edge[1] = nullptr;
            f->disjoin_edge[2] = nullptr;
            f->draw_read = false;
            f->mark = false;
            f->face_type = 0;
            f->is_boundary_face = true;
            return f;
        }

        static bool is_disjoin_face(Face *f1, Face *f2) {
            bool b1 = f1->p1 == f2->p1 || f1->p1 == f2->p2 || f1->p1 == f2->p3;
            bool b2 = f1->p2 == f2->p1 || f1->p2 == f2->p2 || f1->p2 == f2->p3;
            bool b3 = f1->p3 == f2->p1 || f1->p3 == f2->p2 || f1->p3 == f2->p3;
            int i = 0;
            if (b1)
                i++;
            if (b2)
                i++;
            if (b3)
                i++;
            return i == 2;
        }

        static Vector3 get_face_normal(Face *f) {
            //left hand cross
            auto u = f->p2->position - f->p1->position;
            auto v = f->p3->position - f->p1->position;
            auto n = v.cross(u).normalise();
            return n;
        }
    };

    struct Edge {
        Vertex *orig;
        Vertex *end;
        std::vector<Face *> *connect_face_array;
        //for draw debug
        bool draw_red = false;

        static Edge *allocate_from_pool(MemoryPool *pool, Vertex *_orig, Vertex *_end) {
            auto e = (Edge *) pool->allocate();
            e->orig = _orig;
            e->end = _end;

            _orig->connect_edge_array->push_back(e);
            _end->connect_edge_array->push_back(e);
            e->connect_face_array = new std::vector<Face *>();
            return e;
        }

    };

    struct Tetrahedra {
        int type_id;
        int static_index;
        Vertex *p1;
        Vertex *p2;
        Vertex *p3;
        Vertex *p4;

        Tetrahedra *neighbors[4];
        Face *faces[4];
        bool mark;


        //for draw
        bool draw_red = false;


        static Tetrahedra *allocate_from_pool(MemoryPool *pool, Vertex *_p1, Vertex *_p2, Vertex *_p3, Vertex *_p4) {
            auto t = (Tetrahedra *) pool->allocate();
            t->p1 = _p1;
            t->p2 = _p2;
            t->p3 = _p3;
            t->p4 = _p4;
            t->static_index = (*pool).size() - 1;
            t->neighbors[0] = nullptr;
            t->neighbors[1] = nullptr;
            t->neighbors[2] = nullptr;
            t->neighbors[3] = nullptr;

            t->faces[0] = nullptr;
            t->faces[1] = nullptr;
            t->faces[2] = nullptr;
            t->faces[3] = nullptr;

            t->mark = false;
            t->draw_red = false;

            t->p1->connect_tetrahedra_array->push_back(t);
            t->p2->connect_tetrahedra_array->push_back(t);
            t->p3->connect_tetrahedra_array->push_back(t);
            t->p4->connect_tetrahedra_array->push_back(t);

            return t;
        }

        Vertex *get_vtx(int i) {
            if (i == 0)
                return p1;
            if (i == 1)
                return p2;
            if (i == 2)
                return p3;
            if (i == 3)
                return p4;

            assert(false);
            return nullptr;
        }

        static bool is_tetrahedra_disjoin(Tetrahedra *t1, Tetrahedra *t2) {
            int i = 0;
            if (t1->p1 == t2->p1 || t1->p1 == t2->p2 || t1->p1 == t2->p3 || t1->p1 == t2->p4)
                i++;
            if (t1->p2 == t2->p1 || t1->p2 == t2->p2 || t1->p2 == t2->p3 || t1->p2 == t2->p4)
                i++;
            if (t1->p3 == t2->p1 || t1->p3 == t2->p2 || t1->p3 == t2->p3 || t1->p3 == t2->p4)
                i++;
            if (t1->p4 == t2->p1 || t1->p4 == t2->p2 || t1->p4 == t2->p3 || t1->p4 == t2->p4)
                i++;

            return i == 3;

        }

        static int find_disjoin_tet_non_share_vtx_index_in_t2(Tetrahedra *t1, Tetrahedra *t2) {
            bool is_vtx1_share = 0, is_vtx2_share = 0, is_vtx3_share = 0, is_vtx4_share = 0;
            is_vtx1_share = (t2->p1 == t1->p1 || t2->p1 == t1->p2 || t2->p1 == t1->p3 || t2->p1 == t1->p4);
            is_vtx2_share = (t2->p2 == t1->p1 || t2->p2 == t1->p2 || t2->p2 == t1->p3 || t2->p2 == t1->p4);
            is_vtx3_share = (t2->p3 == t1->p1 || t2->p3 == t1->p2 || t2->p3 == t1->p3 || t2->p3 == t1->p4);
            is_vtx4_share = (t2->p4 == t1->p1 || t2->p4 == t1->p2 || t2->p4 == t1->p3 || t2->p4 == t1->p4);

            int i = 0;
            int value = 0;
            if (is_vtx1_share)
                i++;
            else
                value = 0;

            if (is_vtx2_share)
                i++;
            else
                value = 1;

            if (is_vtx3_share)
                i++;
            else
                value = 2;

            if (is_vtx4_share)
                i++;
            else
                value = 3;

            assert(i == 3);

            return value;
        }

        static void bind_tet_and_face(Tetrahedra *t, Face *f) {
            bool b1 = t->p1 == f->p1 || t->p1 == f->p2 || t->p1 == f->p3;
            bool b2 = t->p2 == f->p1 || t->p2 == f->p2 || t->p2 == f->p3;
            bool b3 = t->p3 == f->p1 || t->p3 == f->p2 || t->p3 == f->p3;
            bool b4 = t->p4 == f->p1 || t->p4 == f->p2 || t->p4 == f->p3;
            int i = 0;
            if (b1)
                i++;
            if (b2)
                i++;
            if (b3)
                i++;
            if (b4)
                i++;
            assert(i == 3);
            bool orient;
            if (!b1) {
                orient = Geometrical_Predicates::toleft(f->p1->position, f->p2->position, f->p3->position, t->p1->position);
                t->faces[0] = f;
            }
            if (!b2) {
                orient = toleft(f->p1->position, f->p2->position, f->p3->position, t->p2->position);
                t->faces[1] = f;
            }
            if (!b3) {
                orient = toleft(f->p1->position, f->p2->position, f->p3->position, t->p3->position);
                t->faces[2] = f;
            }
            if (!b4) {
                orient = toleft(f->p1->position, f->p2->position, f->p3->position, t->p4->position);
                t->faces[3] = f;
            }
            orient ? f->disjoin_tet[0] = t : f->disjoin_tet[1] = t;
        }
    };

    struct Triangle_Soup_Mesh {
        MemoryPool vertex_pool;
        MemoryPool edge_pool;
        MemoryPool face_pool;

        Triangle_Soup_Mesh() {
            vertex_pool.initializePool(sizeof(Vertex), 1000, 8, 32);
            edge_pool.initializePool(sizeof(Edge), 1000, 8, 32);
            face_pool.initializePool(sizeof(Face), 1000 * 1.2, 8, 32);
        }


        void clear() {
            vertex_pool.restart();
            edge_pool.restart();
            face_pool.restart();
        }

        bool is_manifold_2() {
            bool b1 = (face_pool.size() * 3 == edge_pool.size() * 2);
            bool b2 = (face_pool.size() - edge_pool.size() + vertex_pool.size() == 2);
            return b1 && b2;
        }

        bool is_closed() {
            for (int i = 0; i < edge_pool.size(); i++) {
                auto e = (Edge *) edge_pool[i];
                if (e->connect_face_array->size() != 2)
                    return false;
            }
            return true;
        }

        void connect_edge_by_face() {
            auto add_edge_if_not_exist = [](MemoryPool &pool, Vertex *a, Vertex *b) -> Edge * {
                for (auto &e: *a->connect_edge_array) {
                    if ((e->end == a && e->orig == b) || (e->orig == a && e->end == b)) {
                        return e;
                    }
                }
                for (auto &e: *b->connect_edge_array) {
                    if ((e->end == a && e->orig == b) || (e->orig == a && e->end == b)) {
                        return e;
                    }
                }
                auto e = Edge::allocate_from_pool(&pool, a, b);

                return e;
            };
            assert(edge_pool.size() == 0);
            for (int i = 0; i < face_pool.size(); i++) {
                Face *f = (Face *) face_pool[i];

                auto e_23 = add_edge_if_not_exist(edge_pool, f->p2, f->p3);
                e_23->connect_face_array->push_back(f);
                f->disjoin_edge[0] = e_23;

                auto e_13 = add_edge_if_not_exist(edge_pool, f->p1, f->p3);
                e_13->connect_face_array->push_back(f);
                f->disjoin_edge[1] = e_13;

                auto e_12 = add_edge_if_not_exist(edge_pool, f->p1, f->p2);
                e_12->connect_face_array->push_back(f);
                f->disjoin_edge[2] = e_12;
            }
        }

        void load_from_file(std::string path, bool check_manifold_2 = false) {

            Mesh_Loader::FileData data;
            Mesh_Loader::load_by_extension(path.c_str(), data);

            std::map<int, int> vtx_relocation_map;
            for (int i = 0; i < data.numberOfPoints; i++) {
                Vector3 p(data.pointList[i * 3], data.pointList[i * 3 + 1], data.pointList[i * 3 + 2]);

                auto find_same_vtx_in_pool = [](MemoryPool &pool, Vector3 p, int &index) -> Vertex * {
                    for (int i = 0; i < pool.size(); i++) {
                        auto v = (Vertex *) pool[i];
                        if (vector_length_sqr(p, v->position) < 1e-8) {
                            index = i;
                            return v;
                        }

                    }
                    return nullptr;
                };
                int index;
                Vertex *vtx_find = find_same_vtx_in_pool(vertex_pool, p, index);
                if (vtx_find == nullptr) {
                    auto v = Vertex::allocate_from_pool(&vertex_pool);
                    v->position = p;
                    vtx_relocation_map[i] = vertex_pool.size() - 1;
                } else {
                    vtx_relocation_map[i] = index;
                }
            }

            for (int i = 0; i < data.numberOfCell; i++) {
                auto cell = data.cellList[i];
                auto p1_index = vtx_relocation_map[cell.pointList[0]];
                auto p2_index = vtx_relocation_map[cell.pointList[1]];
                auto p3_index = vtx_relocation_map[cell.pointList[2]];

                auto p1 = (Vertex *) vertex_pool[p1_index];
                auto p2 = (Vertex *) vertex_pool[p2_index];
                auto p3 = (Vertex *) vertex_pool[p3_index];

                auto find_same_face_in_pool = [](MemoryPool &pool, Vertex *p1, Vertex *p2, Vertex *p3) -> Face * {
                    for (int i = 0; i < pool.size(); i++) {
                        auto f = (Face *) pool[i];
                        bool b1 = f->p1 == p1 || f->p1 == p2 || f->p1 == p3;
                        bool b2 = f->p2 == p1 || f->p2 == p2 || f->p2 == p3;
                        bool b3 = f->p3 == p1 || f->p3 == p2 || f->p3 == p3;
                        if (b1 && b2 && b3)
                            return f;
                    }
                    return nullptr;
                };
                Face *face_find = find_same_face_in_pool(face_pool, p1, p2, p3);//这个地方可以优化，在顶点处保存相连面

                if (face_find == nullptr) {
                    assert(p1 != p2);
                    assert(p1 != p3);
                    assert(p2 != p3);

                    assert(colinear(p1->position, p2->position, p3->position) == false);
                    Face::allocate_from_pool(&face_pool, p1, p2, p3);


                }
            }

            connect_edge_by_face();
        }

        void save(std::string save_path, std::string save_name, bool bSave_vtu = true, bool bSave_obj = false) {
            //save file
            ASSERT_MSG(bSave_vtu || bSave_obj, "set at least one file extension be TRUE");
            using namespace Mesh_Loader;

            FileData data;

            data.numberOfPoints = vertex_pool.size();
            data.pointList = new double[data.numberOfPoints * 3];

            for (int j = 0; j < vertex_pool.size(); j++) {
                const auto &vtx = (Vertex *) vertex_pool[j];
                data.pointList[j * 3] = vtx->position.x;
                data.pointList[j * 3 + 1] = vtx->position.y;
                data.pointList[j * 3 + 2] = vtx->position.z;
            }

            data.numberOfCell = face_pool.size();
            data.cellList = new Cell[data.numberOfCell];

            for (int j = 0; j < face_pool.size(); j++) {
                const auto &f = (Face *) face_pool[j];

                data.cellList[j].pointList = new int[3];
                data.cellList[j].numberOfPoints = 3;

                data.cellList[j].pointList[0] = f->p1->static_index;
                data.cellList[j].pointList[1] = f->p2->static_index;
                data.cellList[j].pointList[2] = f->p3->static_index;
            }


            auto full_path = path_join(save_path, save_name + ".vtu");
            if (bSave_vtu)
                ASSERT_MSG(save_vtu(full_path.c_str(), data), "save fail");
            full_path = path_join(save_path, save_name + ".obj");

            if (bSave_obj)
                ASSERT_MSG(save_obj(full_path.c_str(), data), "save fail");

        }

        double get_wind_number(Vector3 p) {
            double val = 0;
            for (int i = 0; i < face_pool.size(); i++) {
                val += get_wind_number_face_i(p, i);
            }
            return val;
        }

        double get_wind_number_face_i(Vector3 p, int face_index) {
            double val = 0;

            auto f = (Face *) face_pool[face_index];
            auto a = f->p1->position - p;
            auto b = f->p2->position - p;
            auto c = f->p3->position - p;

            auto a_len = vector_length(a);
            auto b_len = vector_length(b);
            auto c_len = vector_length(c);

            auto ab_dot = dot(a, b);
            auto bc_dot = dot(b, c);
            auto ca_dot = dot(c, a);

            double a11, a12, a13;
            double a21, a22, a23;
            double a31, a32, a33;

            a11 = a.x;
            a12 = b.x;
            a13 = c.x;
            a21 = a.y;
            a22 = b.y;
            a23 = c.y;
            a31 = a.z;
            a32 = b.z;
            a33 = c.z;

            double det_abc = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a31 * a22 * a13 - a32 * a23 * a11 - a21 * a12 * a33;

            double denominator = a_len * b_len * c_len + ab_dot * c_len + bc_dot * a_len + ca_dot * b_len;

            val += 2 * atan2(det_abc, denominator);

            return val;
        }
    };


    struct Tetrahedra_Mesh {
        MemoryPool vertex_pool;
        MemoryPool tet_pool;

        Tetrahedra_Mesh() {
            vertex_pool.initializePool(sizeof(Vertex), 1000, 8, 32);
            tet_pool.initializePool(sizeof(Tetrahedra), 1000 * 1.2, 8, 32);
        }

        void clear() {
            vertex_pool.restart();
            tet_pool.restart();
        }
        void load_from_file(std::string path){
            Mesh_Loader::FileData data;
            Mesh_Loader::load_by_extension(path.c_str(), data);

        }

        void save(std::string save_path, std::string save_name) {
            //save file

            using namespace Mesh_Loader;

            FileData data;

            data.numberOfPoints = vertex_pool.size();
            data.pointList = new double[data.numberOfPoints * 3];

            for (int j = 0; j < vertex_pool.size(); j++) {
                const auto &vtx = (Vertex *) vertex_pool[j];
                data.pointList[j * 3] = vtx->position.x;
                data.pointList[j * 3 + 1] = vtx->position.y;
                data.pointList[j * 3 + 2] = vtx->position.z;
            }

            data.numberOfCell = tet_pool.size();
            data.cellList = new Cell[data.numberOfCell];

            for (int j = 0; j < tet_pool.size(); j++) {
                const auto &t = (Tetrahedra *) tet_pool[j];

                data.cellList[j].pointList = new int[4];
                data.cellList[j].numberOfPoints = 4;

                assert(t->p1->static_index>=0);
                assert(t->p2->static_index>=0);
                assert(t->p3->static_index>=0);
                assert(t->p4->static_index>=0);

                data.cellList[j].pointList[0] = t->p1->static_index;
                data.cellList[j].pointList[1] = t->p2->static_index;
                data.cellList[j].pointList[2] = t->p3->static_index;
                data.cellList[j].pointList[3] = t->p4->static_index;
            }

            auto full_path = path_join(save_path, save_name + "_normal_fix" + ".vtu");
            ASSERT_MSG(save_vtu(full_path.c_str(), data), "save fail");

        }


    };

}

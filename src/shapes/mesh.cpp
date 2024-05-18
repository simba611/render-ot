#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
 * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class TriangleMesh : public AccelerationStructure {
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
     * vertex index (into @c m_vertices ) of the triangle.
     * This list will always contain as many elements as there are triangles.
     */
    std::vector<Vector3i> m_triangles;
    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
     * vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging purposes.
    std::filesystem::path m_originalPath;
    /// @brief Whether to interpolate the normals from m_vertices, or report the geometric normal instead.
    bool m_smoothNormals;

protected:
    int numberOfPrimitives() const override {
        return int(m_triangles.size());
    }

    inline void populate(SurfaceEvent &surf, const Point &position, const Vector &norm, const Vector2 &bary, const Vertex &vert1, const Vertex &vert2, const Vertex &vert3) const {
            surf.position = position;
            
            Vertex inter = Vertex().interpolate(bary, vert1, vert2, vert3);

            surf.uv.x() = inter.texcoords[0];
            surf.uv.y() = inter.texcoords[1];
 
            if (!m_smoothNormals) {
                surf.frame.normal = norm.normalized();
            }
            else {
                surf.frame.normal = (inter.normal).normalized();
            }

            Frame new_frame = Frame(surf.frame.normal);
            surf.frame = new_frame;

            // set to 0 for assignment 1
            surf.pdf = 0.f;
        }

    bool intersect(int primitiveIndex, const Ray &ray, Intersection &its, Sampler &rng) const override {
        Vector direction = ray.direction;
        Point origin = ray.origin;

        const Vector3i tri_ind = m_triangles[primitiveIndex];

        const Vertex vert1 = m_vertices[tri_ind[0]];
        const Vertex vert2 = m_vertices[tri_ind[1]];
        const Vertex vert3 = m_vertices[tri_ind[2]];

        const Point v1 = vert1.position;
        const Point v2 = vert2.position;
        const Point v3 = vert3.position;

        // Check if its a valid triangle
        if ((v1 == v2) || (v1 == v3) || (v2 == v3)) {
            return false;
        }

        Vector e1 = Vector(v2) - Vector(v1);
        Vector e2 = Vector(v3) - Vector(v1);
        Vector e1xe2 = e1.cross(e2);
        
        // Check if ray is parallel to triangle
        if (e1xe2.dot(direction) == 0) {
            return false;
        }

        // Cramer's rule 
        float det, t, u, v;        
        Vector b = Vector(origin) - Vector(v1);

        Matrix3x3 A = Matrix3x3();
        A.setColumn(0, -direction);
        A.setColumn(1, e1);
        A.setColumn(2, e2);
        det = A.determinant();

        Matrix3x3 A1 = Matrix3x3();
        A1.setColumn(0, b);
        A1.setColumn(1, e1);
        A1.setColumn(2, e2);

        t = A1.determinant() / det;

        if ((t < Epsilon) || (its.t < t)) {
            return false;
        }

        Matrix3x3 A2 = Matrix3x3();
        A2.setColumn(0, -direction);
        A2.setColumn(1, b);
        A2.setColumn(2, e2);

        u = A2.determinant() / det;

        if ((u < 0) || (u > 1)) {
            return false;
        }

        Matrix3x3 A3 = Matrix3x3();
        A3.setColumn(0, -direction);
        A3.setColumn(1, e1);
        A3.setColumn(2, b);

        v = A3.determinant() / det;

        if ((v < 0) || ((1 - u - v) < 0)) {
            return false;
        }

        its.t = t;

        const Point position = ray(t);
        populate(its, position, e1xe2, Vector2(u, v), vert1, vert2, vert3);

        return true;
        // hints:
        // * use m_triangles[primitiveIndex] to get the vertex indices of the triangle that should be intersected
        // * if m_smoothNormals is true, interpolate the vertex normals from m_vertices
        //   * make sure that your shading frame stays orthonormal!
        // * if m_smoothNormals is false, use the geometrical normal (can be computed from the vertex positions)
    }

    Bounds getBoundingBox(int primitiveIndex) const override {
        const Vector3i tri_ind = m_triangles[primitiveIndex];

        const Vertex vert1 = m_vertices[tri_ind[0]];
        const Vertex vert2 = m_vertices[tri_ind[1]];
        const Vertex vert3 = m_vertices[tri_ind[2]];

        const Point v1 = vert1.position;
        const Point v2 = vert2.position;
        const Point v3 = vert3.position;

        Bounds b;
        b.extend(v1);
        b.extend(v2);
        b.extend(v3);

        return b;
    }

    Point getCentroid(int primitiveIndex) const override {
        const Vector3i tri_ind = m_triangles[primitiveIndex];

        const Vertex vert1 = m_vertices[tri_ind[0]];
        const Vertex vert2 = m_vertices[tri_ind[1]];
        const Vertex vert3 = m_vertices[tri_ind[2]];

        const Point v1 = vert1.position;
        const Point v2 = vert2.position;
        const Point v3 = vert3.position;

        return (Vector(v1) + Vector(v2) + Vector(v3)) / 3;
    }

public:
    TriangleMesh(const Properties &properties) {
        m_originalPath = properties.get<std::filesystem::path>("filename");
        m_smoothNormals = properties.get<bool>("smooth", true);
        readPLY(m_originalPath.string(), m_triangles, m_vertices);
        logger(EInfo, "loaded ply with %d triangles, %d vertices",
            m_triangles.size(),
            m_vertices.size()
        );
        buildAccelerationStructure();
    }

    AreaSample sampleArea(Sampler &rng) const override {
        // only implement this if you need triangle mesh area light sampling for your rendering competition
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return tfm::format(
            "Mesh[\n"
            "  vertices = %d,\n"
            "  triangles = %d,\n"
            "  filename = \"%s\"\n"
            "]",
            m_vertices.size(),
            m_triangles.size(),
            m_originalPath.generic_string()
        );
    }
};

}

REGISTER_SHAPE(TriangleMesh, "mesh")

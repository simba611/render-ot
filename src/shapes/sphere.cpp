#include <lightwave.hpp>

namespace lightwave

{
    class Sphere : public Shape
    {

    inline void populate(SurfaceEvent &surf, const Point &position) const {
        surf.position = ((Vector)position).normalized();
        
        // NOT_IMPLEMENTED

        Vector cv(0.f, 0.f, 0.f);
        cv[Vector(position).minComponentIndex()] = 1.0f;
        
        // normal at a point for on a sphere with centre (cx, cy, cz) will just be the direction of the point from centre
        surf.frame.normal = Vector(position).normalized();
         
        // tangent is always parallel to the surface. for sphere, any vector perpendicular to the normal is tangent
        surf.frame.tangent = surf.frame.normal.cross(cv).normalized();

        // bitagent is always orthogonal to both the normal and the tangent. according to convention, its defined as normal X tangent
        surf.frame.bitangent = surf.frame.normal.cross(surf.frame.tangent);

        surf.uv.x() = 0.5 + (atan2(-surf.frame.normal.x(), -surf.frame.normal.z()) / (2 * Pi));
        surf.uv.y() = 0.5 + (asin(-surf.frame.normal.y()) / Pi);

        // set to 0 for assignment 1
        surf.pdf = 1 / (4 * Pi);
    }
    
    public:
        Sphere(const Properties &properties) {
        }
        bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override {
            Point o = ray.origin;
            Vector d = ray.direction;

            const float a = d.lengthSquared();
            const float b = 2 * d.dot(Vector(o));
            const float c = Vector(o).lengthSquared() - 1;

            const float det = (b * b) - (4 * a * c);
            const float det_r = sqrt(det);

            if (det < 0)
                return false;

            float t = (-b - det_r) / (2 * a);
            if (t < Epsilon)
                t = (-b + det_r) / (2 * a);
            
            if (t < Epsilon || t > its.t) // note: t can still be less than 0, but t < Epsilon checks for t < 0 already
                return false;

            its.t = t;

            const Point position = ray(t);
            populate(its, position);

            return true;
        } 
        Bounds getBoundingBox() const override{
            return Bounds(Point { -1, -1, -1 }, Point { +1, +1, +1 });
        } 
        Point getCentroid() const override{
            return Point(0.f, 0.f, 0.f);
        } 
        AreaSample sampleArea(Sampler &rng) const override{
            Vector p = Vector(rng.next(), rng.next(), rng.next()).normalized();

            AreaSample sample;
            populate(sample, p);
            return sample;
        } 
            std::string toString() const override
        {
            return "Sphere[]";
        }
    };
}

REGISTER_SHAPE(Sphere, "sphere")
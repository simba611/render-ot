#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {

    Vector trans_tangent = (m_transform -> apply(surf.frame.tangent)).normalized();

    if (m_flipNormal) {
        surf.frame.bitangent *= Vector(-1.f, -1.f, -1.f);
    }
    
    Vector trans_bitangent = (m_transform -> apply(surf.frame.bitangent)).normalized();
    Vector n_normal = (trans_tangent.cross(trans_bitangent)).normalized();
    Vector n_bitangent = (n_normal.cross(trans_tangent)).normalized();

    surf.frame.normal = n_normal;
    surf.frame.tangent = trans_tangent;
    surf.frame.bitangent = n_bitangent; 
}

bool Instance::intersect(const Ray &worldRay, Intersection &its, Sampler &rng) const {
    if (!m_transform) {
        // fast path, if no transform is needed
        Ray localRay = worldRay;
        if (m_shape->intersect(localRay, its, rng)) {
            its.instance = this;
            return true;
        } else {
            return false;
        }
        return false;
    }

    const float previousT = its.t;
    Ray trans_ray = m_transform -> inverse(worldRay);
    const float norm_factor = trans_ray.direction.length();

    Ray localRay = trans_ray.normalized();
    its.t = previousT * norm_factor;

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        its.t /= norm_factor;
        its.instance = this;
        transformFrame(its);
        return true;
    } else {
        its.t = previousT;
        return false;
    }
}

Bounds Instance::getBoundingBox() const {
    if (!m_transform) {
        // fast path
        return m_shape->getBoundingBox();
    }

    const Bounds untransformedAABB = m_shape->getBoundingBox();
    if (untransformedAABB.isUnbounded()) {
        return Bounds::full();
    }

    Bounds result;
    for (int point = 0; point < 8; point++) {
        Point p = untransformedAABB.min();
        for (int dim = 0; dim < p.Dimension; dim++) {
            if ((point >> dim) & 1) {
                p[dim] = untransformedAABB.max()[dim];
            }
        }
        p = m_transform->apply(p);
        result.extend(p);
    }
    return result;
}

Point Instance::getCentroid() const {
    if (!m_transform) {
        // fast path
        return m_shape->getCentroid();
    }

    return m_transform->apply(m_shape->getCentroid());
}

AreaSample Instance::sampleArea(Sampler &rng) const {
    AreaSample sample = m_shape->sampleArea(rng);
    transformFrame(sample);
    return sample;
}

}

REGISTER_CLASS(Instance, "instance", "default")

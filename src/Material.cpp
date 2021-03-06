#include "Material.h"
#include <glm/gtc/type_ptr.hpp> 
#include <Texture.h>
#include <Math/Frame.h>
#include <Math/Random.h>
#include <Pdf.h>

Material::~Material()
{
}

glm::vec3 Material::emitted(const Math::Ray& in, const HitRecord& rec) const
{
	return glm::vec3(0.f);
}

Lambertian::~Lambertian()
{
}

bool Lambertian::scatter(AreaAlloc& alloc, const Math::Ray& in, const HitRecord& rec, ScatterRecord& srec) const 
{
	srec.bSpecular = false;
	srec.attenuation = m_Albedo->value(rec.u, rec.v, rec.position);
	srec.pdf_ptr = std::allocate_shared<CosinePdf>(alloc, rec.normal);
    return true;
}

float Lambertian::scatteringPdf(const Math::Ray& in, const HitRecord& rec, const Math::Ray& scattered) const
{
	const auto pi = glm::pi<float>();

	float cosine = glm::dot(rec.normal, scattered.direction());
    cosine = glm::max(cosine, 0.f);
	return cosine / pi;
}

Metal::~Metal()
{
}

bool Metal::scatter(AreaAlloc& alloc, const Math::Ray& in, const HitRecord& rec, ScatterRecord& srec) const
{
    glm::vec3 reflected = glm::reflect(in.direction(), rec.normal);
	glm::vec3 dir = glm::normalize(reflected + m_Fuzz*Math::randomUnitSphere());
    srec.specular_ray = Math::Ray(rec.position, dir, in.time());
    srec.attenuation = m_Albedo->value(rec.u, rec.v, rec.position);
	srec.bSpecular = true;
	srec.pdf_ptr = nullptr;

    return glm::dot(reflected, rec.normal) > 0;
}


Dielectric::~Dielectric()
{
}

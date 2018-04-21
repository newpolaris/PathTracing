#include "Atmosphere.h"
#include <glm/gtc/constants.hpp>
#include <algorithm>

glm::vec2 RaySphereIntersect(glm::vec3 pos, glm::vec3 dir, glm::vec3 c, float r)
{
	assert(r > 0.f);

	glm::vec3 tc = c - pos;

	float l = glm::dot(tc, dir);
	float d = l*l - glm::dot(tc, tc) + r*r;
	if (d < 0) return glm::vec2(-1.0);
	float sl = glm::sqrt(d);

	return glm::vec2(l - sl, l + sl);
}

Atmosphere::Atmosphere(glm::vec3 sunDir) : 
	m_SunDir(sunDir)
{
}

glm::vec4 Atmosphere::computeIncidentLight(const glm::vec3& pos, const glm::vec3& dir, float tmin, float tmax) const
{
	const glm::vec3 SunIntensity = glm::vec3(20.f);
	const int numSamples = 16;
	const int numLightSamples = 8;
    const float g = 0.76f; 
	const float pi = glm::pi<float>();

	auto t = RaySphereIntersect(pos, dir, m_Ec, m_Ar);
	tmin = std::max(t.x, 0.f);
	tmax = t.y;
	if (tmax < 0) return glm::vec4(0.f);
	auto tc = pos;
	auto pa = tc + tmax*dir, pb = tc + tmin*dir;

	float opticalDepthR = 0.f, opticalDepthM = 0.f;
	float ds = (tmax - tmin) / numSamples; // delta segment
	glm::vec3 sumR(0.f), sumM(0.f);
	for (int s = 0; s < numSamples; s++)
	{
		glm::vec3 x = pb + ds*(0.5f + s)*dir;
		float h = glm::length(x) - m_Er;
		float betaR = glm::exp(-h/m_Hr)*ds;
        float betaM = glm::exp(-h/m_Hm)*ds;
		opticalDepthR += betaR;
        opticalDepthM += betaM;
		auto tl = RaySphereIntersect(x, m_SunDir, m_Ec, m_Ar);
		float lmax = tl.y, lmin = 0.f;
		float dls = (lmax - lmin)/numLightSamples; // delta light segment
		int l = 0;
		float opticalDepthLightR = 0.f, opticalDepthLightM = 0.f;
		for (; l < numLightSamples; l++)
		{
			glm::vec3 xl = x + dls*(0.5f + l)*m_SunDir;
			float hl = glm::length(xl) - m_Er;
			if (hl < 0) break;
			opticalDepthLightR += glm::exp(-hl/m_Hr)*dls;
			opticalDepthLightM += glm::exp(-hl/m_Hm)*dls;
		}
		if (l < numLightSamples) continue;
        glm::vec3 tauR = m_BetaR0 * (opticalDepthR + opticalDepthLightR);
        glm::vec3 tauM = 1.1f * m_BetaM0 * (opticalDepthM + opticalDepthLightM);
		glm::vec3 tau = tauR + tauM;
		glm::vec3 attenuation = glm::exp(-tau);
		sumR += attenuation * betaR;
        sumM += attenuation * betaM;
	}

	float mu = glm::dot(m_SunDir, dir);
	float phaseR = 3.f / (16.f*pi) * (1.f + mu*mu);
	float phaseM = 3.f / (8.f*pi) * ((1 - g*g)*(1 + mu*mu))/((2 + g*g)*pow(1 + g*g - 2*g*mu, 1.5f));
    glm::vec3 color = sumR * phaseR * m_BetaR0 + sumM * phaseM * m_BetaM0;
	return glm::vec4(SunIntensity * color, 1.f);
}

void Atmosphere::renderSkyDome(std::vector<glm::vec4>& image, int width, int height) const
{
	const float inf = 9e8;
	const glm::vec3 cameraPos(0.f, m_Er+1.f, 0.f);

	for (int y = 0; y < height; y++)
	for (int x = 0; x < height; x++)
	{
		float fy = 2.f * ((float)y + 0.5f)/(height-1) - 1.f;
		float fx = 2.f * ((float)x + 0.5f)/(height-1) - 1.f;
		float z2 = 1.f - (fy*fy+fx*fx);
		if (z2 < 0) continue;
		// glm::vec3 dir = glm::normalize(glm::vec3(0.5f, 0.5f, 0.f));
		glm::vec3 dir = glm::normalize(glm::vec3(fx, glm::sqrt(z2), fy));
		image[y*width + x] = computeIncidentLight(cameraPos, dir, 0.f, inf);
	}
}
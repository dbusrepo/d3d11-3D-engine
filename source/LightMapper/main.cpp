#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <vector>

#include "Image.h"
#include "Vector.h"
#include "TexturePacker.h"
#include "BSP.h"

#define INPUT_FILE_PATH "G:\\Documenti\\GoogleDrive\\D3DGraphicsProgramming\\D3D11Projects\\D3DEngine\\source\\BSP_PVS_Compiler\\LightMapGenInfo.dat"
#define LIGHT_INNER_RADIUS 50.f
#define LIGHT_RADIUS 1300.f
#define LIGHTMAP_RES_FACTOR 8.f
#define CLUSTER_DIV_FACTOR 2

using ClusterMapType = uint16_t;

struct Light {
	float origin[3];
};

struct BoundingRect {
	float3 origin;
	float3 uDir, vDir;
	float width, height;
};

int main() {
	
	/********************************/
	// READ INPUT DATA FROM FILE
	/********************************/
	FILE *inFile;

	if ((inFile = fopen(INPUT_FILE_PATH, "rb")) == 0) {
		printf("File %s NOT found. Exiting...\n", INPUT_FILE_PATH);
		exit(1);
	}

	uint16_t numLights;
	fread(&numLights, sizeof(uint16_t), 1, inFile);

	std::vector<Light> lightsVec;

	for (uint16_t i = 0; i != numLights; ++i)
	{
		Light light;
		fread(&light.origin, sizeof(float), 3, inFile);
		lightsVec.push_back(light);
	}

	uint32_t numFloats; // num floats to read

	uint32_t numPolygons;
	fread(&numPolygons, sizeof(uint32_t), 1, inFile);
	uint32_t *polygonsFirstVertexVec = new uint32_t[numPolygons];
	fread(polygonsFirstVertexVec, sizeof(uint32_t), numPolygons, inFile);

	// polygon normals array
	fread(&numFloats, sizeof(uint32_t), 1, inFile);
	float *polygonNormArry = new float[numFloats];
	fread(polygonNormArry, sizeof(float), numFloats, inFile);
	
	// vertices array
	fread(&numFloats, sizeof(uint32_t), 1, inFile);
	float *vertexPosArr = new float[numFloats];
	fread(vertexPosArr, sizeof(float), numFloats, inFile);
	uint32_t numVertices = numFloats / 3;

	// num floats for vertices uv coords
	fread(&numFloats, sizeof(uint32_t), 1, inFile);
	float *vertexUvArr = new float[numFloats];
	fread(vertexUvArr, sizeof(float), numFloats, inFile);

	uint32_t numIndices;
	fread(&numIndices, sizeof(uint32_t), 1, inFile);
	uint32_t *indexArr = new uint32_t[numIndices];
	fread(indexArr, sizeof(uint32_t), numIndices, inFile);

	fclose(inFile);
	/********************************/
	// END READ INPUT DATA FROM FILE
	/********************************/

	const float3* polygonsNormalsArray = reinterpret_cast<float3*>(polygonNormArry);
	const float3* src = reinterpret_cast<float3*>(vertexPosArr);
	const float2* srcUv = reinterpret_cast<float2*>(vertexUvArr);
	const uint32_t* inds = indexArr;
	

	TexturePacker texPacker;
	BoundingRect *boundingRectArr = new BoundingRect[numPolygons];


	for (int iPoly = 0; iPoly != numPolygons; ++iPoly) {
		uint32_t firstVertexIdx = polygonsFirstVertexVec[iPoly];
		uint32_t endVertexIdx = (iPoly != numPolygons - 1) ?
								polygonsFirstVertexVec[iPoly + 1] : numVertices;

		float3 v0 = src[firstVertexIdx];
		float3 v1 = src[firstVertexIdx+1];
		float3 v2 = src[firstVertexIdx+2];
		float2 uv0 = srcUv[firstVertexIdx];
		float2 uv1 = srcUv[firstVertexIdx+1];
		float2 uv2 = srcUv[firstVertexIdx+2];

		float3 edge1 = v1 - v0;
		float3 edge2 = v2 - v0;
		float2 deltaUV1 = uv1 - uv0;
		float2 deltaUV2 = uv2 - uv0;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		float3 tangent, bitangent;

		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent = normalize(tangent);

		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent = normalize(bitangent);

		float3 u = tangent;
		float3 v = bitangent;
		float3 d;

		float uMin = 0;
		float uMax = 0;
		float vMin = 0;
		float vMax = 0;
		float3 origin = src[firstVertexIdx];

		for (uint32_t i = firstVertexIdx; i < endVertexIdx; i++)
		{
			d = src[i] - origin;
			float uLength = dot(d,u);
			float vLength = dot(d,v);
			uMin = min(uLength, uMin);
			uMax = max(uLength, uMax);
			vMin = min(vLength, vMin);
			vMax = max(vLength, vMax);
		}

		//uMin -= 0.5f;
		//vMin -= 0.5f;
		//uMax += 0.5f;
		//vMax += 0.5f;
		
		d = u * uMin;
		origin += d;
		d = v * vMin;
		origin += d;

		float width = uMax - uMin;
		float height = vMax - vMin;

		boundingRectArr[iPoly].origin = origin;
		boundingRectArr[iPoly].uDir = tangent;
		boundingRectArr[iPoly].vDir = bitangent;
		boundingRectArr[iPoly].width = width;
		boundingRectArr[iPoly].height = height;

		float w = width / LIGHTMAP_RES_FACTOR;//28;
		float h = height / LIGHTMAP_RES_FACTOR;//28;

		if (w < 8) w = 8;
		if (h < 8) h = 8;

		uint tw = (int)w;
		uint th = (int)h;

		tw = (tw + 4) & ~7;
		th = (th + 4) & ~7;

		texPacker.addRectangle(tw, th);
		
	}

	uint lm_width = 1024;//512;
	uint lm_height = 1024;//512;

	if (!texPacker.assignCoords(&lm_width, &lm_height, widthComp))
	{
		ErrorMsg("Lightmap too small");
		return false;
	}

	BSP m_BSP;

	float2* lmCoords = new float2[numVertices];
	uint* lmIndices = new uint[numIndices];

	uint32_t rectIdx = 0;
	uint32_t lmCoordsIdx = 0;
	uint32_t indicesIDx = 0;

	for (int iPoly = 0; iPoly != numPolygons; ++iPoly) {
		uint32_t firstVertexIdx = polygonsFirstVertexVec[iPoly];
		uint32_t endVertexIdx = (iPoly != numPolygons - 1) ?
			polygonsFirstVertexVec[iPoly + 1] : numVertices;

		TextureRectangle* rect = texPacker.getRectangle(rectIdx++);

		float3 origin = boundingRectArr[iPoly].origin;
		float3 u = boundingRectArr[iPoly].uDir;
		float3 v = boundingRectArr[iPoly].vDir;

		for (uint32_t i = firstVertexIdx; i < endVertexIdx; i++)
		{
			float3 d = src[i] - origin;
			float uLength = dot(d, u) / boundingRectArr[iPoly].width;
			float vLength = dot(d, v) / boundingRectArr[iPoly].height;

			float xl = float(rect->x + uLength * rect->width) / lm_width;
			float yl = float(rect->y + vLength * rect->height) / lm_height;

			lmCoords[lmCoordsIdx++] = float2(xl, yl);

			if (i - firstVertexIdx >= 2) {
				float3 v0 = src[inds[indicesIDx]];
				float3 v1 = src[inds[indicesIDx + 1]];
				float3 v2 = src[inds[indicesIDx + 2]];
				indicesIDx += 3;
				m_BSP.addTriangle(v0, v1, v2);
			}
		}
	}

	/*
	uint32_t index = 0;
	for (uint i = 0; i < numIndices; i += 6)
	{
		TextureRectangle* rect = texPacker.getRectangle(index);

		float x0 = float(rect->x + 0.5f) / lm_width;
		float y0 = float(rect->y + 0.5f) / lm_height;
		float x1 = float(rect->x + rect->width - 0.5f) / lm_width;
		float y1 = float(rect->y + rect->height - 0.5f) / lm_height;

		lmCoords[4 * index + 0] = float2(x0, y0);
		lmCoords[4 * index + 1] = float2(x1, y0);
		lmCoords[4 * index + 2] = float2(x1, y1);
		lmCoords[4 * index + 3] = float2(x0, y1);

		lmIndices[i + 0] = 4 * index;
		lmIndices[i + 1] = 4 * index + 1;
		lmIndices[i + 2] = 4 * index + 2;
		lmIndices[i + 3] = 4 * index;
		lmIndices[i + 4] = 4 * index + 2;
		lmIndices[i + 5] = 4 * index + 3;

		float3 v0 = src[inds[i]];
		float3 v1 = src[inds[i + 1]];
		float3 v2 = src[inds[i + 2]];
		float3 v3 = src[inds[i + 5]];

		m_BSP.addTriangle(v0, v1, v2);
		m_BSP.addTriangle(v0, v2, v3);

		index++;
	}
	*/

	m_BSP.build();

	/*****	Lightmap generation code *****/
	const uint SAMPLE_COUNT = 250;

	float3 p_samples[SAMPLE_COUNT];
	float2 t_samples[SAMPLE_COUNT];
	for (uint s = 0; s < SAMPLE_COUNT; s++)
	{
		float3 p;
		do
		{
			p = float3(float(rand()), float(rand()), float(rand())) * (2.0f / RAND_MAX) - 1.0f;
		} while (dot(p, p) > 1.0f);
		p_samples[s] = p;

		t_samples[s] = float2(float(rand()), float(rand())) * (1.0f / RAND_MAX) - 0.5f;
	}

	/* Cluster Map setup up */
	const uint cluster_divisor = CLUSTER_DIV_FACTOR;
	const uint cm_width = lm_width / cluster_divisor;
	const uint cm_height = lm_height / cluster_divisor;
	ClusterMapType* clusters = new ClusterMapType[cm_width * cm_height];
	memset(clusters, 0, cm_width * cm_height * sizeof(ClusterMapType));
	/* End Cluster Map setup up */

	// Lightmapping. 
	for (uint light_index = 0; light_index < numLights; light_index++)
	{
		const float3 light_pos = *(reinterpret_cast<float3*>(&lightsVec[light_index].origin));

		uint8* lMap = new uint8[lm_width * lm_height];
		memset(lMap, 0, lm_width * lm_height);

		uint32_t rectIdx = 0;
		for (int iPoly = 0; iPoly != numPolygons; ++iPoly) {

			const TextureRectangle& rect = *texPacker.getRectangle(rectIdx++);

			float3 pos = boundingRectArr[iPoly].origin;
			float3 dirS = boundingRectArr[iPoly].uDir * boundingRectArr[iPoly].width;
			float3 dirT = boundingRectArr[iPoly].vDir * boundingRectArr[iPoly].height;
			//float3 normal = cross(dirS, dirT);
			float3 normal = polygonsNormalsArray[iPoly];
			float d = -dot(pos, normal);
			if (dot(light_pos, normal) + d < 0.0f)
				continue;

			// Compute light contribution on this rectangle
			for (uint t = 0; t < rect.height; t++)
			{
				for (uint s = 0; s < rect.width; s++)
				{
					float3 lumelHd = ((float(s)+.5f) / rect.width) * dirS;
					float3 lumelVd = ((float(t)+.5f) / rect.height) * dirT;
					float3 lumelPos = pos + lumelHd + lumelVd;
					float dist = length(light_pos - lumelPos);

					if (dist > LIGHT_RADIUS) continue;

					float light = 0.0f;
					for (uint k = 0; k < SAMPLE_COUNT; k++)
					{
						float3 hd = saturate( (((float(s) + .5f) / (rect.width)) + t_samples[k].x / (rect.width)) ) * dirS;
						float3 vd = saturate( (((float(t) + .5f) / (rect.height)) + t_samples[k].y / (rect.height)) ) * dirT;
						float3 samplePos = pos + hd + vd;

						m_BSP.pushSphere(samplePos, 0.3f);
						m_BSP.pushSphere(samplePos, 0.2f);
						m_BSP.pushSphere(samplePos, 0.1f);
						
						float3 lightSample = light_pos + LIGHT_INNER_RADIUS * p_samples[k];
						
						if (!m_BSP.intersects(samplePos, lightSample)) {
							light += 1.0f / SAMPLE_COUNT;
						}
					}

					uint8 light_val = (uint8)(255.0f * sqrtf(light) + 0.5f); // sqrtf() for poor man's gamma
					lMap[(rect.y + t) * lm_width + (rect.x + s)] = light_val;
				}
			}

			// Assign light clusters for this rectangle
			const uint s0 = rect.x / cluster_divisor;
			const uint t0 = rect.y / cluster_divisor;
			const uint s1 = s0 + rect.width / cluster_divisor;
			const uint t1 = t0 + rect.height / cluster_divisor;
			for (uint t = t0; t < t1; t++)
			{
				for (uint s = s0; s < s1; s++)
				{
					// Sample the light within the rect, plus one extra pixel border to cover filtering, but clamp to stay within the rect
					uint ls0 = max(int(s * cluster_divisor - 1), (int)rect.x);
					uint ls1 = min((s + 1) * cluster_divisor + 1, rect.x + rect.width);
					uint lt0 = max(int(t * cluster_divisor - 1), (int)rect.y);
					uint lt1 = min((t + 1) * cluster_divisor + 1, rect.y + rect.height);

					for (uint lt = lt0; lt < lt1; lt++)
					{
						for (uint ls = ls0; ls < ls1; ls++)
						{
							const uint8 light_val = lMap[lt * lm_width + ls];
							if (light_val)
							{
								clusters[t * cm_width + s] |= (1 << light_index);
								goto double_break;
							}
						}
					}
				double_break:
					;
				}
			}
		}

		/*
		uint index = 0;
		for (uint i = 0; i < numIndices; i += 6)
		{
			const TextureRectangle& rect = *texPacker.getRectangle(index);
			index++;

			float3 pos = src[inds[i]];

			float3 dirS = (src[inds[i + 1]] - pos);
			float3 dirT = (src[inds[i + 5]] - pos);
			float3 normal = cross(dirS, dirT);

			float d = -dot(pos, normal);

			if (dot(light_pos, normal) + d < 0.0f)
				continue;

			// Compute light constribution on this rectangle
			for (uint t = 0; t < rect.height; t++)
			{
				for (uint s = 0; s < rect.width; s++)
				{
					float light = 0.0f;
					for (uint k = 0; k < SAMPLE_COUNT; k++)
					{
						float3 samplePos = pos +
							saturate((float(s) / (rect.width - 1)) + t_samples[k].x / (rect.width - 1)) * dirS +
							saturate((float(t) / (rect.height - 1)) + t_samples[k].y / (rect.height - 1)) * dirT;

						m_BSP.pushSphere(samplePos, 0.3f);
						m_BSP.pushSphere(samplePos, 0.2f);
						m_BSP.pushSphere(samplePos, 0.1f);

						if (!m_BSP.intersects(samplePos, light_pos + 150.0f * p_samples[k]))
							light += 1.0f / SAMPLE_COUNT;
					}

					uint8 light_val = (uint8)(255.0f * sqrtf(light) + 0.5f); // sqrtf() for poor man's gamma
					lMap[(rect.y + t) * lm_width + (rect.x + s)] = light_val;
				}
			}

			// Assign light clusters for this rectangle
			const uint s0 = rect.x / cluster_divisor;
			const uint t0 = rect.y / cluster_divisor;
			const uint s1 = s0 + rect.width / cluster_divisor;
			const uint t1 = t0 + rect.height / cluster_divisor;
			for (uint t = t0; t < t1; t++)
			{
				for (uint s = s0; s < s1; s++)
				{
					// Sample the light within the rect, plus one extra pixel border to cover filtering, but clamp to stay within the rect
					uint ls0 = max(int(s * cluster_divisor - 1), (int)rect.x);
					uint ls1 = min((s + 1) * cluster_divisor + 1, rect.x + rect.width);
					uint lt0 = max(int(t * cluster_divisor - 1), (int)rect.y);
					uint lt1 = min((t + 1) * cluster_divisor + 1, rect.y + rect.height);

					for (uint lt = lt0; lt < lt1; lt++)
					{
						for (uint ls = ls0; ls < ls1; ls++)
						{
							const uint8 light_val = lMap[lt * lm_width + ls];
							if (light_val)
							{
								clusters[t * cm_width + s] |= (1 << light_index);
								goto double_break;
							}
						}
					}
				double_break:
					;
				}
			}
		}
		*/

		Image image;
		image.loadFromMemory(lMap, FORMAT_I8, lm_width, lm_height, 1, 1, true);
		char fileName[256];
		sprintf(fileName, "LightMap%d.dds", light_index);
		image.saveImage(fileName);

	}

	Image image;
	image.loadFromMemory(clusters, FORMAT_R16UI, cm_width, cm_height, 1, 1, true);
	image.saveImage("Clusters.dds");

	system("pause");

	return 0;
}
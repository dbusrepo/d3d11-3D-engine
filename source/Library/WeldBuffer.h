#pragma once

template <typename VertexMaterialType>
size_t WeldBuffersT(byte * pMem, size_t numVertices, ULONG * pIndices, size_t numIndices)
{
	struct RelocInfo {
		bool relocated;
		size_t newIndex;
		std::vector<size_t> *relocatedIndices;
	};

	RelocInfo *remap = new RelocInfo[numVertices];
	memset(remap, 0, sizeof(RelocInfo) * numVertices);

	VertexMaterialType *vertices = reinterpret_cast<VertexMaterialType*>(pMem);

	for (size_t i = 0; i != numVertices; i++) {
		if (!remap[i].relocated) {
			remap[i].relocatedIndices = new std::vector<size_t>;
			const VertexMaterialType &curVertex = vertices[i];
			for (size_t j = i + 1; j != numVertices; j++) {
				if (curVertex.same(vertices[j])) {
					remap[j].relocated = true;
					remap[i].relocatedIndices->push_back(j);
				}
			}
		}
	}

	size_t f = 0;
	// vertices[0..i-1] rearranged: vertices[0,f-1] good, vertices[f..i-1] free, 0 <= f <= i <= numVertices
	for (size_t i = 0; i != numVertices; i++) {
		if (!remap[i].relocated) {
			vertices[f] = vertices[i];
			remap[i].newIndex = f;
			const std::vector<size_t> &relocatedIndices = *remap[i].relocatedIndices;
			for (auto it = cbegin(relocatedIndices); it != cend(relocatedIndices); ++it) {
				size_t relocatedIndex = *it;
				remap[relocatedIndex].newIndex = f;
			}
			++f;
		}
	}

	size_t numFinalVertices = f;

	// remap indices
	for (size_t i = 0; i != numIndices; ++i) {
		pIndices[i] = (ULONG)remap[pIndices[i]].newIndex;
	}

	for (size_t i = 0; i != numVertices; i++) {
		if (!remap[i].relocated) {
			delete remap[i].relocatedIndices;
			remap[i].relocatedIndices = nullptr;
		}
	}

	delete[] remap;

	return numFinalVertices;
}

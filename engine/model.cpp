#include "model.h"
#include "app.h"
const aiScene * qb::ModelMgr::getAssimpScene(const std::string name) {
	// read assimp scene from cache
	auto it = _aiSceneMap.find(name);
	if (it != _aiSceneMap.end())
		return it->second;
	const aiScene* scene = _importer.ReadFile(name,
		aiProcess_Triangulate | 
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs
	);
	if (scene == nullptr) {
		log_error("import model error:%s", _importer.GetErrorString());
		assert(0);
	}
	_aiSceneMap.insert({ name, scene });

	return _aiSceneMap.at(name);
}
qb::Model * qb::ModelMgr::getModel(const std::string name) {
	// read model from cache
	auto it = _modelMap.find(name);
	if (it != _modelMap.end())
		return it->second;
	Model* model = new Model();
	model->init(app, name);
	_modelMap.insert({ name, model });
	return _modelMap.at(name);
}
void qb::ModelMgr::init(App *app) {
	this->app = app;
}


void qb::ModelMgr::destroy() {

}

void qb::Model::init(App * app, std::string name){
	this->app = app;
	this->name = name;
}

void qb::Model::destroy(){

}

void qb::Model::build(){
	assert(scene != nullptr);
	// animation
	for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
		std::string name(scene->mAnimations[i]->mName.data);
		animIdMaps[name] = i;
	}
	// bone
	uint32_t vertexCount = 0;
	for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
		vertexCount += scene->mMeshes[m]->mNumVertices;
	}
	vertexBoneDatas.resize(vertexCount);
	globalInverseTransform = scene->mRootNode->mTransformation;
	globalInverseTransform.Inverse();
	uint32_t vertexBase = 0;
	for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
		aiMesh* mesh = scene->mMeshes[m];
		if (mesh->mNumBones > 0) {
			loadBones(mesh, vertexBase, vertexBoneDatas);
		}
		vertexBase += scene->mMeshes[m]->mNumVertices;
	}
	// model vertex
	vertexBase = 0;
	modelVertices = {};
	for (size_t m = 0; m < scene->mNumMeshes; m++) {
		for (size_t v = 0; v < scene->mMeshes[m]->mNumVertices; v++) {
			ModelVertex modelVertex;
			modelVertex.pos = glm::make_vec3(&scene->mMeshes[m]->mVertices[v].x);
			modelVertex.color = (scene->mMeshes[m]->HasVertexColors(0)) ?
				glm::make_vec3(&scene->mMeshes[m]->mColors[0][v].r) : glm::vec3(1.0f);
			modelVertex.texCoord = glm::make_vec2(&scene->mMeshes[m]->mTextureCoords[0][v].x);
			modelVertex.normal = glm::make_vec3(&scene->mMeshes[m]->mNormals[v].x);

			for (uint32_t j = 0; j < max_bones_per_vertex; j++) {
				modelVertex.boneWeights[j] = vertexBoneDatas[vertexBase + v].weights[j];
				modelVertex.boneIDs[j] = vertexBoneDatas[vertexBase + v].ids[j];
			}

			modelVertices.push_back(modelVertex);
		}
		vertexBase += scene->mMeshes[m]->mNumVertices;
	}
	
	app->bufferMgr.destroyBuffer("$modelVertexBuf_" + name);
	modelVertexBuf = app->bufferMgr.getBuffer("$modelVertexBuf_" + name);
	modelVertexBuf->bufferInfo.size = sizeof(ModelVertex) * modelVertices.size();
	modelVertexBuf->bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	modelVertexBuf->build();
	modelVertexBuf->mapping(modelVertices.data());
	// model index
	modelIndices = {};
	for (size_t m = 0; m < scene->mNumMeshes; m++) {
		size_t indexBase = modelIndices.size();
		for (size_t f = 0; f < scene->mMeshes[m]->mNumFaces; f++) {
			for (size_t i = 0; i < 3; i++) {
				modelIndices.push_back(scene->mMeshes[m]->mFaces[f].mIndices[i] + static_cast<uint32_t>(indexBase));
			}
		}
	}
	app->bufferMgr.destroyBuffer("$modelIndexBuf_" + name);
	modelIndexBuf = app->bufferMgr.getBuffer("$modelIndexBuf_" + name);
	modelIndexBuf->bufferInfo.size = sizeof(uint16_t) * modelIndices.size();
	modelIndexBuf->bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	modelIndexBuf->build();
	modelIndexBuf->mapping(modelIndices.data());
}

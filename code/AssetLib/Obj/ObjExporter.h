/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2020, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file ObjExporter.h
 * Declares the exporter class to write a scene to a Collada file
 */
#ifndef AI_OBJEXPORTER_H_INC
#define AI_OBJEXPORTER_H_INC

#include <assimp/types.h>
#include <sstream>
#include <vector>
#include <map>

struct aiScene;
struct aiNode;
struct aiMesh;

namespace Assimp {

// ------------------------------------------------------------------------------------------------
/** Helper class to export a given scene to an OBJ file. */
// ------------------------------------------------------------------------------------------------
class ObjExporter {
public:
    /// Constructor for a specific scene to export
	//要导出的特定场景的构造函数
    ObjExporter(const char* filename, const aiScene* pScene, bool noMtl=false);
    ~ObjExporter();
    std::string GetMaterialLibName();//获取材质名
    std::string GetMaterialLibFileName();//获取材质名
    
    /// public string-streams to write all output into
	//public 字符串输出写出所有output
    std::ostringstream mOutput, mOutputMat;//mOutputMat材质文件

private:
    // intermediate data structures //中间的数据结构
    struct FaceVertex { //面顶点
        FaceVertex()
        : vp()
        , vn()
        , vt() {
            // empty
        }

        // one-based, 0 means: 'does not exist'
        unsigned int vp, vn, vt; //面的：顶点、法线、纹理的索引
    };

    struct Face { //面
        char kind;//类型：f面、p线、l点
        std::vector<FaceVertex> indices; //三个顶点信息（三角面的话），obj支持三个点以上的面
    };

    struct MeshInstance { //mesh索引
        std::string name, matname;//name是mesh名字；matname为纹理坐标名（如果没有材质的时候使用）
        std::vector<Face> faces;//面构成mesh
    };

    void WriteHeader(std::ostringstream& out);//文件首部声明，包括版本、来源等
    void WriteMaterialFile();//写出mtl文件
    void WriteGeometryFile(bool noMtl=false);//写出几何文件
    std::string GetMaterialName(unsigned int index);//获得材质名，如“building.mtl”
    void AddMesh(const aiString& name, const aiMesh* m, const aiMatrix4x4& mat);//添加mesh
    void AddNode(const aiNode* nd, const aiMatrix4x4& mParent);//添加node

private:
    std::string filename;//文件名
    const aiScene* const pScene;//场景

    struct vertexData {  //顶点数据：一个向量，一个颜色
        aiVector3D vp;//顶点
        aiColor3D vc; // OBJ does not support 4D color//obj不支持4d颜色
    };

    std::vector<aiVector3D> vn, vt; //vn法向；vt贴图坐标
    std::vector<aiColor4D> vc; //顶点颜色
    std::vector<vertexData> vp;//顶点坐标
    bool useVc; //是否使用顶点颜色

    struct vertexDataCompare {
        bool operator() ( const vertexData& a, const vertexData& b ) const {
            // position
            if (a.vp.x < b.vp.x) return true;
            if (a.vp.x > b.vp.x) return false;
            if (a.vp.y < b.vp.y) return true;
            if (a.vp.y > b.vp.y) return false;
            if (a.vp.z < b.vp.z) return true;
            if (a.vp.z > b.vp.z) return false;

            // color
            if (a.vc.r < b.vc.r) return true;
            if (a.vc.r > b.vc.r) return false;
            if (a.vc.g < b.vc.g) return true;
            if (a.vc.g > b.vc.g) return false;
            if (a.vc.b < b.vc.b) return true;
            if (a.vc.b > b.vc.b) return false;
            return false;
        }
    };

    struct aiVectorCompare { 
        bool operator() (const aiVector3D& a, const aiVector3D& b) const { 
            if(a.x < b.x) return true; 
            if(a.x > b.x) return false; 
            if(a.y < b.y) return true; 
            if(a.y > b.y) return false; 
            if(a.z < b.z) return true; 
            return false;
        }
    };

    template <class T, class Compare = std::less<T>> //类模板，建立一个T与int类型对应的map类。//键为T，值为int，map第三个元素为compate
    class indexMap {
        int mNextIndex;
        typedef std::map<T, int, Compare> dataType;//值为int类型
        dataType vecMap;
    
    public:
        indexMap()
        : mNextIndex(1) {
            // empty
        }

        int getIndex(const T& key) { //根据键找值
            typename dataType::iterator vertIt = vecMap.find(key);
            // vertex already exists, so reference it
            if(vertIt != vecMap.end()){
                return vertIt->second;
            }
            return vecMap[key] = mNextIndex++;//返回当前索引位置
        };

        void getKeys( std::vector<T>& keys ) {
            keys.resize(vecMap.size());
            for(typename dataType::iterator it = vecMap.begin(); it != vecMap.end(); ++it){
                keys[it->second-1] = it->first;
            }
        };
    };

    indexMap<aiVector3D, aiVectorCompare> mVnMap, mVtMap;
    indexMap<vertexData, vertexDataCompare> mVpMap;//三维点map？//vertexData包含6个元素{vc，vp}，对应的值为其排序位置？
    std::vector<MeshInstance> mMeshes;//mesh列表

    // this endl() doesn't flush() the stream
    const std::string endl;
};

}

#endif

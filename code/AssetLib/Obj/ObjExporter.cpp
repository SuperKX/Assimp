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

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_OBJ_EXPORTER

#include "ObjExporter.h"
#include <assimp/Exceptional.h>
#include <assimp/StringComparison.h>
#include <assimp/version.h>
#include <assimp/IOSystem.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <memory>

using namespace Assimp;

namespace Assimp {

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to Wavefront OBJ. Prototyped and registered in Exporter.cpp //在Exporter.cpp中创建及声明
void ExportSceneObj(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/) {
    // invoke the exporter
    ObjExporter exporter(pFile, pScene); //构造函数的时候完成输出。114行。

	//以下为异常判断
    if (exporter.mOutput.fail() || exporter.mOutputMat.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));//输出数据创建失败，最可能是文件太大导致的。
    }

    // we're still here - export successfully completed. Write both the main OBJ file and the material script
	//成功完成obj和mtl文件写出
    {
        std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
        if (outfile == nullptr) {
            throw DeadlyExportError("could not open output .obj file: " + std::string(pFile));//无法打开obj文件
        }
        outfile->Write( exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()),1);
    }
    {
        std::unique_ptr<IOStream> outfile (pIOSystem->Open(exporter.GetMaterialLibFileName(),"wt"));
        if (outfile == nullptr) {
            throw DeadlyExportError("could not open output .mtl file: " + std::string(exporter.GetMaterialLibFileName()));//无法打开mtl文件
        }
        outfile->Write( exporter.mOutputMat.str().c_str(), static_cast<size_t>(exporter.mOutputMat.tellp()),1);
    }
}

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to Wavefront OBJ without the material file. Prototyped and registered in Exporter.cpp
void ExportSceneObjNoMtl(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* ) {
    // invoke the exporter
    ObjExporter exporter(pFile, pScene, true);

    if (exporter.mOutput.fail() || exporter.mOutputMat.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));
    }

    // we're still here - export successfully completed. Write both the main OBJ file and the material script
    {
        std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
        if (outfile == nullptr) {
            throw DeadlyExportError("could not open output .obj file: " + std::string(pFile));
        }
        outfile->Write( exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()),1);
    }


}

} // end of namespace Assimp

static const std::string MaterialExt = ".mtl";

// ------------------------------------------------------------------------------------------------
ObjExporter::ObjExporter(const char* _filename, const aiScene* pScene, bool noMtl)
: filename(_filename)
, pScene(pScene)
, vn()
, vt()
, vp()
, useVc(false)
, mVnMap()
, mVtMap()
, mVpMap()
, mMeshes()
, endl("\n") {
    // make sure that all formatting happens using the standard, C locale and not the user's current locale
    const std::locale& l = std::locale("C");
    mOutput.imbue(l);
    mOutput.precision(ASSIMP_AI_REAL_TEXT_PRECISION);
    mOutputMat.imbue(l);
    mOutputMat.precision(ASSIMP_AI_REAL_TEXT_PRECISION);

    WriteGeometryFile(noMtl);//写出几何文件
    if ( !noMtl ) {
        WriteMaterialFile();//写出材质文件
    }
}

// ------------------------------------------------------------------------------------------------
ObjExporter::~ObjExporter() {
    // empty
}

// ------------------------------------------------------------------------------------------------
std::string ObjExporter::GetMaterialLibName() {//找到相同地址
    // within the Obj file, we use just the relative file name with the path stripped
    const std::string& s = GetMaterialLibFileName();
    std::string::size_type il = s.find_last_of("/\\");//查找最后一个斜杠反斜杠位置。
    if (il != std::string::npos) {
        return s.substr(il + 1);//返回最后一个斜杠后
    }

    return s;
}

// ------------------------------------------------------------------------------------------------
std::string ObjExporter::GetMaterialLibFileName() { //找到原始文件名，返回同名的mtl后缀名。
    // Remove existing .obj file extension so that the final material file name will be fileName.mtl and not fileName.obj.mtl
    size_t lastdot = filename.find_last_of('.');
    if ( lastdot != std::string::npos ) {
        return filename.substr( 0, lastdot ) + MaterialExt;
    }

    return filename + MaterialExt;
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::WriteHeader(std::ostringstream& out) { //文件头部写相关注释
    out << "# File produced by Open Asset Import Library (http://www.assimp.sf.net)" << endl;
    out << "# (assimp v" << aiGetVersionMajor() << '.' << aiGetVersionMinor() << '.'
        << aiGetVersionRevision() << ")" << endl  << endl;
}

// ------------------------------------------------------------------------------------------------
std::string ObjExporter::GetMaterialName(unsigned int index) {
    const aiMaterial* const mat = pScene->mMaterials[index];
    if ( nullptr == mat ) {
        static const std::string EmptyStr;
        return EmptyStr;
    }

    aiString s;
    if(AI_SUCCESS == mat->Get(AI_MATKEY_NAME,s)) {
        return std::string(s.data,s.length);
    }

    char number[ sizeof(unsigned int) * 3 + 1 ];
    ASSIMP_itoa10(number,index);
    return "$Material_" + std::string(number);
}

// ------------------------------------------------------------------------------------------------
//参考《Obj模型之mtl文件格式》：https://www.jianshu.com/p/afa7ffa01191
void ObjExporter::WriteMaterialFile() {//写出材质文件
    WriteHeader(mOutputMat);//材质文件头注释

    for(unsigned int i = 0; i < pScene->mNumMaterials; ++i) {//逐个材质处理
        const aiMaterial* const mat = pScene->mMaterials[i];//获取材质文件

        int illum = 1;//照明度，1表示Color on and Ambient on
        mOutputMat << "newmtl " << GetMaterialName(i)  << endl;//材质文件名

        aiColor4D c;//向量类型的值
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE,c)) {	//散射光（diffuse color）用Kd
            mOutputMat << "Kd " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_AMBIENT,c)) {	//材质的环境光（ambient color）
            mOutputMat << "Ka " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_SPECULAR,c)) {	//镜面光（specular color）用Ks
            mOutputMat << "Ks " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_EMISSIVE,c)) { //放射光
            mOutputMat << "Ke " << c.r << " " << c.g << " " << c.b << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_TRANSPARENT,c)) {	//滤光透射率
            mOutputMat << "Tf " << c.r << " " << c.g << " " << c.b << endl;
        }

        ai_real o;//float类型的值
        if(AI_SUCCESS == mat->Get(AI_MATKEY_OPACITY,o)) {	//渐隐指数
            mOutputMat << "d " << o << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_REFRACTI,o)) {	//折射值描述
            mOutputMat << "Ni " << o << endl;
        }

        if(AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS,o) && o) {	//反射指数
            mOutputMat << "Ns " << o << endl;
            illum = 2; //照明度，2表示Highlight on
        }

        mOutputMat << "illum " << illum << endl; //照明度

		//以下是mtl文件中对于纹理映射的描述格式
        aiString s;
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_DIFFUSE(0),s)) {//为漫反射指定颜色纹理文件
            mOutputMat << "map_Kd " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_AMBIENT(0),s)) {//为环境反射指定颜色纹理文件
            mOutputMat << "map_Ka " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_SPECULAR(0),s)) {//为镜反射指定颜色纹理文件
            mOutputMat << "map_Ks " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_SHININESS(0),s)) {
            mOutputMat << "map_Ns " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_OPACITY(0),s)) {
            mOutputMat << "map_d " << s.data << endl;
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_HEIGHT(0),s) || AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE_NORMALS(0),s)) {
            // implementations seem to vary here, so write both variants
            mOutputMat << "bump " << s.data << endl;
            mOutputMat << "map_bump " << s.data << endl;
        }

        mOutputMat << endl;
    }
}

void ObjExporter::WriteGeometryFile(bool noMtl) {
    WriteHeader(mOutput);//文件头部写相关注释
    if (!noMtl)//构造时默认值为无材质
        mOutput << "mtllib "  << GetMaterialLibName() << endl << endl; //写出mtl文件名

    // collect mesh geometry
    aiMatrix4x4 mBase;
    AddNode(pScene->mRootNode, mBase);//递归构造整个scene的节点结构//这里是根据mesh列表构造mode

    // write vertex positions with colors, if any
    mVpMap.getKeys( vp );//输出顶点坐标（如果包含颜色同时输出颜色）
    if ( !useVc ) { //不适用顶点颜色
        mOutput << "# " << vp.size() << " vertex positions" << endl;
        for ( const vertexData& v : vp ) {
            mOutput << "v  " << v.vp.x << " " << v.vp.y << " " << v.vp.z << endl;
        }
    } else {
        mOutput << "# " << vp.size() << " vertex positions and colors" << endl;
        for ( const vertexData& v : vp ) {
            mOutput << "v  " << v.vp.x << " " << v.vp.y << " " << v.vp.z << " " << v.vc.r << " " << v.vc.g << " " << v.vc.b << endl;
        }
    }
    mOutput << endl;

    // write uv coordinates//写出uv坐标
    mVtMap.getKeys(vt);
    mOutput << "# " << vt.size() << " UV coordinates" << endl;
    for(const aiVector3D& v : vt) {
        mOutput << "vt " << v.x << " " << v.y << " " << v.z << endl;
    }
    mOutput << endl;

    // write vertex normals//写出顶点法向坐标
    mVnMap.getKeys(vn);
    mOutput << "# " << vn.size() << " vertex normals" << endl;
    for(const aiVector3D& v : vn) {
        mOutput << "vn " << v.x << " " << v.y << " " << v.z << endl;
    }
    mOutput << endl;

    // now write all mesh instances  //写出所有mesh索引
    for(const MeshInstance& m : mMeshes) {//逐个mesh处理
        mOutput << "# Mesh \'" << m.name << "\' with " << m.faces.size() << " faces" << endl;
        if (!m.name.empty()) { //mesh名不为空时输出名字
            mOutput << "g " << m.name << endl;
        }
        if ( !noMtl ) {//若无材质则输出材质名
            mOutput << "usemtl " << m.matname << endl;//纹理坐标名称
        }

        for(const Face& f : m.faces) {  //逐个面写出：“f  1//1 2//2 3//3”
            mOutput << f.kind << ' ';//该单元面类型信息
            for(const FaceVertex& fv : f.indices) {//逐个面索引写出
                mOutput << ' ' << fv.vp;//面顶点

                if (f.kind != 'p') {
                    if (fv.vt || f.kind == 'f') {//如果有vt信息，或者类型是“f”，添加“/”
                        mOutput << '/';
                    }
                    if (fv.vt) {//写出vt（注意这里逻辑，f一定要添加“/”）
                        mOutput << fv.vt;
                    }
                    if (f.kind == 'f' && fv.vn) {//如果有vn信息，写出vn
                        mOutput << '/' << fv.vn;
                    }
                }
            }

            mOutput << endl;
        }
        mOutput << endl;
    }
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::AddMesh(const aiString& name, const aiMesh* m, const aiMatrix4x4& mat) {
    mMeshes.push_back(MeshInstance() );//构造一个新的mesh放入mMeshes列表中
    MeshInstance& mesh = mMeshes.back();//最后一个mesh的索引

    if ( nullptr != m->mColors[ 0 ] ) {	//mColors存在
        useVc = true;//使用顶点颜色
    }

    mesh.name = std::string( name.data, name.length );//mesh名
    mesh.matname = GetMaterialName(m->mMaterialIndex);//mesh的材质名称

    mesh.faces.resize(m->mNumFaces);//设置mesh面数量

    for(unsigned int i = 0; i < m->mNumFaces; ++i) {//mesh中逐个面处理，赋值面到mesh中
        const aiFace& f = m->mFaces[i];//assimp中定义的aiFace

        Face& face = mesh.faces[i];//objexporter中定义的face
        switch (f.mNumIndices) {//复制face中类型
            case 1:
                face.kind = 'p';//点
                break;
            case 2:
                face.kind = 'l';//线
                break;
            default:
                face.kind = 'f';//面
        }
        face.indices.resize(f.mNumIndices);//设置顶点数量

        for(unsigned int a = 0; a < f.mNumIndices; ++a) {
            const unsigned int idx = f.mIndices[a];//逐个顶点索引赋值

            aiVector3D vert = mat * m->mVertices[idx];//计算全局下顶点坐标

            if ( nullptr != m->mColors[ 0 ] ) {//顶点颜色存在
                aiColor4D col4 = m->mColors[ 0 ][ idx ];
                face.indices[a].vp = mVpMap.getIndex({vert, aiColor3D(col4.r, col4.g, col4.b)});//根据键找值，这里的键是6个元素的vertexData
            } else {
                face.indices[a].vp = mVpMap.getIndex({vert, aiColor3D(0,0,0)});
            }

            if (m->mNormals) { //面法向存在
                aiVector3D norm = aiMatrix3x3(mat) * m->mNormals[idx];
                face.indices[a].vn = mVnMap.getIndex(norm);
            } else {
                face.indices[a].vn = 0;
            }

            if ( m->mTextureCoords[ 0 ] ) {  //面纹理坐标存在
                face.indices[a].vt = mVtMap.getIndex(m->mTextureCoords[0][idx]);
            } else {
                face.indices[a].vt = 0;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void ObjExporter::AddNode(const aiNode* nd, const aiMatrix4x4& mParent) {//递归添加节点，及其子节点。
    const aiMatrix4x4& mAbs = mParent * nd->mTransformation;//到此节点时累计的变换矩阵（相当于将局部相对变换改成全局坐标下的变换）

    aiMesh *cm( nullptr );//cm指向“nd所指向节点的”mesh
	for (unsigned int i = 0; i < nd->mNumMeshes; ++i) {//md节点逐个mesh处理
		cm = pScene->mMeshes[nd->mMeshes[i]];//cm=nd指向的mesh列表中的某个mesh
		if (nullptr != cm) {
			AddMesh(cm->mName, pScene->mMeshes[nd->mMeshes[i]], mAbs);
		}
		else {	//nd节点的某个mesh不存在？
			AddMesh(nd->mName, pScene->mMeshes[nd->mMeshes[i]], mAbs);//输入节点名字（如果节点mesh找不到，用节点名字替代mesh，名）
		}
	}

    for(unsigned int i = 0; i < nd->mNumChildren; ++i) {	//如果有子节点，将会再次查看子节点
        AddNode(nd->mChildren[i], mAbs);
    }
}

// ------------------------------------------------------------------------------------------------

#endif // ASSIMP_BUILD_NO_OBJ_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT

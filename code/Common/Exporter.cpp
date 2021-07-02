/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2020, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

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
---------------------------------------------------------------------------
*/

/** @file Exporter.cpp

Assimp export interface. While it's public interface bears many similarities
to the import interface (in fact, it is largely symmetric), the internal
implementations differs a lot. Exporters are considered stateless and are
simple callbacks which we maintain in a global list along with their
description strings.

Here we implement only the C++ interface (Assimp::Exporter).
*/

#ifndef ASSIMP_BUILD_NO_EXPORT

#include <assimp/BlobIOSystem.h>
#include <assimp/SceneCombiner.h>
#include <assimp/DefaultIOSystem.h>
#include <assimp/Exporter.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Exceptional.h>

#include "Common/DefaultProgressHandler.h"
#include "Common/BaseProcess.h"
#include "Common/ScenePrivate.h"
#include "PostProcessing/CalcTangentsProcess.h"
#include "PostProcessing/MakeVerboseFormat.h"
#include "PostProcessing/JoinVerticesProcess.h"
#include "PostProcessing/ConvertToLHProcess.h"
#include "PostProcessing/PretransformVertices.h"

#include <memory>

namespace Assimp {

#ifdef _MSC_VER
#    pragma warning( disable : 4800 )
#endif // _MSC_VER


// PostStepRegistry.cpp
void GetPostProcessingStepInstanceList(std::vector< BaseProcess* >& out);

// ------------------------------------------------------------------------------------------------
// Exporter worker function prototypes. Do not use const, because some exporter need to convert 
// the scene temporary  //Exporter辅助函数原型
//函数声明
#ifndef ASSIMP_BUILD_NO_COLLADA_EXPORTER
void ExportSceneCollada(const char*,IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_X_EXPORTER
void ExportSceneXFile(const char*,IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_STEP_EXPORTER
void ExportSceneStep(const char*,IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_OBJ_EXPORTER
void ExportSceneObj(const char*,IOSystem*, const aiScene*, const ExportProperties*);
void ExportSceneObjNoMtl(const char*,IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_STL_EXPORTER
void ExportSceneSTL(const char*,IOSystem*, const aiScene*, const ExportProperties*);
void ExportSceneSTLBinary(const char*,IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_PLY_EXPORTER
void ExportScenePly(const char*,IOSystem*, const aiScene*, const ExportProperties*);
void ExportScenePlyBinary(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_3DS_EXPORTER
void ExportScene3DS(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_GLTF_EXPORTER
void ExportSceneGLTF(const char*, IOSystem*, const aiScene*, const ExportProperties*);
void ExportSceneGLB(const char*, IOSystem*, const aiScene*, const ExportProperties*);
void ExportSceneGLTF2(const char*, IOSystem*, const aiScene*, const ExportProperties*);
void ExportSceneGLB2(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_ASSBIN_EXPORTER
void ExportSceneAssbin(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_ASSXML_EXPORTER
void ExportSceneAssxml(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_X3D_EXPORTER
void ExportSceneX3D(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_FBX_EXPORTER
void ExportSceneFBX(const char*, IOSystem*, const aiScene*, const ExportProperties*);
void ExportSceneFBXA(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_3MF_EXPORTER
void ExportScene3MF( const char*, IOSystem*, const aiScene*, const ExportProperties* );
#endif
#ifndef ASSIMP_BUILD_NO_M3D_EXPORTER
void ExportSceneM3D(const char*, IOSystem*, const aiScene*, const ExportProperties*);
void ExportSceneM3DA(const char*, IOSystem*, const aiScene*, const ExportProperties*);
#endif
#ifndef ASSIMP_BUILD_NO_ASSJSON_EXPORTER
void ExportAssimp2Json(const char* , IOSystem*, const aiScene* , const Assimp::ExportProperties*);
#endif

static void setupExporterArray(std::vector<Exporter::ExportFormatEntry> &exporters) {  //捕捉全部exporter
	(void)exporters;//种防止编译器编译时报警告，告诉编译器该变量已经使用了。
					//exporters是存放ExportFormatEntry结构体的vector

#ifndef ASSIMP_BUILD_NO_COLLADA_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("collada", "COLLADA - Digital Asset Exchange Schema", "dae", &ExportSceneCollada));
#endif

#ifndef ASSIMP_BUILD_NO_X_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("x", "X Files", "x", &ExportSceneXFile,
			aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs));
#endif

#ifndef ASSIMP_BUILD_NO_STEP_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("stp", "Step Files", "stp", &ExportSceneStep, 0));
#endif

#ifndef ASSIMP_BUILD_NO_OBJ_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("obj", "Wavefront OBJ format", "obj", &ExportSceneObj,
			aiProcess_GenSmoothNormals /*| aiProcess_PreTransformVertices */));
	exporters.push_back(Exporter::ExportFormatEntry("objnomtl", "Wavefront OBJ format without material file", "obj", &ExportSceneObjNoMtl,
			aiProcess_GenSmoothNormals /*| aiProcess_PreTransformVertices */));
#endif

#ifndef ASSIMP_BUILD_NO_STL_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("stl", "Stereolithography", "stl", &ExportSceneSTL,
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_PreTransformVertices));
	exporters.push_back(Exporter::ExportFormatEntry("stlb", "Stereolithography (binary)", "stl", &ExportSceneSTLBinary,
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_PreTransformVertices));
#endif

#ifndef ASSIMP_BUILD_NO_PLY_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("ply", "Stanford Polygon Library", "ply", &ExportScenePly,
			aiProcess_PreTransformVertices));
	exporters.push_back(Exporter::ExportFormatEntry("plyb", "Stanford Polygon Library (binary)", "ply", &ExportScenePlyBinary,
			aiProcess_PreTransformVertices));
#endif

#ifndef ASSIMP_BUILD_NO_3DS_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("3ds", "Autodesk 3DS (legacy)", "3ds", &ExportScene3DS,
			aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices));
#endif

#if !defined(ASSIMP_BUILD_NO_GLTF_EXPORTER) && !defined(ASSIMP_BUILD_NO_GLTF2_EXPORTER)
	exporters.push_back(Exporter::ExportFormatEntry("gltf2", "GL Transmission Format v. 2", "gltf", &ExportSceneGLTF2,
			aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType));
	exporters.push_back(Exporter::ExportFormatEntry("glb2", "GL Transmission Format v. 2 (binary)", "glb", &ExportSceneGLB2,
			aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType));
#endif

#if !defined(ASSIMP_BUILD_NO_GLTF_EXPORTER) && !defined(ASSIMP_BUILD_NO_GLTF1_EXPORTER)
	exporters.push_back(Exporter::ExportFormatEntry("gltf", "GL Transmission Format", "gltf", &ExportSceneGLTF,
			aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType));
	exporters.push_back(Exporter::ExportFormatEntry("glb", "GL Transmission Format (binary)", "glb", &ExportSceneGLB,
			aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType));
#endif

#ifndef ASSIMP_BUILD_NO_ASSBIN_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("assbin", "Assimp Binary File", "assbin", &ExportSceneAssbin, 0));
#endif

#ifndef ASSIMP_BUILD_NO_ASSXML_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("assxml", "Assimp XML Document", "assxml", &ExportSceneAssxml, 0));
#endif

#ifndef ASSIMP_BUILD_NO_X3D_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("x3d", "Extensible 3D", "x3d", &ExportSceneX3D, 0));
#endif

#ifndef ASSIMP_BUILD_NO_FBX_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("fbx", "Autodesk FBX (binary)", "fbx", &ExportSceneFBX, 0));
	exporters.push_back(Exporter::ExportFormatEntry("fbxa", "Autodesk FBX (ascii)", "fbx", &ExportSceneFBXA, 0));
#endif

#ifndef ASSIMP_BUILD_NO_M3D_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("m3d", "Model 3D (binary)", "m3d", &ExportSceneM3D, 0));
	exporters.push_back(Exporter::ExportFormatEntry("m3da", "Model 3D (ascii)", "a3d", &ExportSceneM3DA, 0));
#endif

#ifndef ASSIMP_BUILD_NO_3MF_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("3mf", "The 3MF-File-Format", "3mf", &ExportScene3MF, 0));
#endif

#ifndef ASSIMP_BUILD_NO_ASSJSON_EXPORTER
	exporters.push_back(Exporter::ExportFormatEntry("assjson", "Assimp JSON Document", "json", &ExportAssimp2Json, 0));
#endif
}

class ExporterPimpl {
public:
    ExporterPimpl()
    : blob()
    , mIOSystem(new Assimp::DefaultIOSystem())
    , mIsDefaultIOHandler(true)
    , mProgressHandler( nullptr )
    , mIsDefaultProgressHandler( true )
    , mPostProcessingSteps()
    , mError()
    , mExporters()//用于存放exporter
	{
        GetPostProcessingStepInstanceList(mPostProcessingSteps);//获得后处理步骤实例表

        // grab all built-in exporters
		setupExporterArray(mExporters);//调用函数，捕捉全部exporter
    }

    ~ExporterPimpl() {
        delete blob;

        // Delete all post-processing plugins//删除所有后处理插件
        for( unsigned int a = 0; a < mPostProcessingSteps.size(); a++) {
            delete mPostProcessingSteps[a];//存放BaseProcess的vector
        }
        delete mProgressHandler;
    }

public:
    aiExportDataBlob* blob; //导出数据的数据块
    std::shared_ptr< Assimp::IOSystem > mIOSystem;//指向mIOSystem的指针
    bool mIsDefaultIOHandler;

    /** The progress handler */
    ProgressHandler *mProgressHandler;  //过程处理类
    bool mIsDefaultProgressHandler;

    /** Post processing steps we can apply at the imported data. */ //可对输入数据进行 后处理
    std::vector< BaseProcess* > mPostProcessingSteps;//#include "Common/BaseProcess.h"//后处理掩码

    /** Last fatal export error */  //最后输出致命错
    std::string mError;

    /** Exporters, this includes those registered using #Assimp::Exporter::RegisterExporter */
    std::vector<Exporter::ExportFormatEntry> mExporters; //存放导出格式选项（ExportFormatEntry）结构体的vector。
	//mExporters是一个ExportFormatEntry的vector，定义在Exporter类中的结构体。
};

} // end of namespace Assimp

using namespace Assimp;

// ------------------------------------------------------------------------------------------------
Exporter :: Exporter()
: pimpl(new ExporterPimpl()) {
    pimpl->mProgressHandler = new DefaultProgressHandler();
}  //Exporter构造同时，给pimpl初始化

// ------------------------------------------------------------------------------------------------
Exporter::~Exporter() {
	ai_assert(nullptr != pimpl);
	FreeBlob();
    delete pimpl;
}

// ------------------------------------------------------------------------------------------------
void Exporter::SetIOHandler( IOSystem* pIOHandler) {
	ai_assert(nullptr != pimpl);
	pimpl->mIsDefaultIOHandler = !pIOHandler;
    pimpl->mIOSystem.reset(pIOHandler);
}

// ------------------------------------------------------------------------------------------------
IOSystem* Exporter::GetIOHandler() const {
	ai_assert(nullptr != pimpl);
	return pimpl->mIOSystem.get();
}

// ------------------------------------------------------------------------------------------------
bool Exporter::IsDefaultIOHandler() const {
	ai_assert(nullptr != pimpl);
	return pimpl->mIsDefaultIOHandler;
}

// ------------------------------------------------------------------------------------------------
void Exporter::SetProgressHandler(ProgressHandler* pHandler) {
    ai_assert(nullptr != pimpl);

    if ( nullptr == pHandler) {
        // Release pointer in the possession of the caller
        pimpl->mProgressHandler = new DefaultProgressHandler();
        pimpl->mIsDefaultProgressHandler = true;
        return;
    }

    if (pimpl->mProgressHandler == pHandler) {
        return;
    }

    delete pimpl->mProgressHandler;
    pimpl->mProgressHandler = pHandler;
    pimpl->mIsDefaultProgressHandler = false;
}

// ------------------------------------------------------------------------------------------------
const aiExportDataBlob* Exporter::ExportToBlob( const aiScene* pScene, const char* pFormatId,
                                                unsigned int pPreprocessing, const ExportProperties* pProperties) {
	ai_assert(nullptr != pimpl);
    if (pimpl->blob) {
        delete pimpl->blob;
        pimpl->blob = nullptr;
    }

    std::shared_ptr<IOSystem> old = pimpl->mIOSystem;
    BlobIOSystem* blobio = new BlobIOSystem();
    pimpl->mIOSystem = std::shared_ptr<IOSystem>( blobio );

    if (AI_SUCCESS != Export(pScene,pFormatId,blobio->GetMagicFileName(), pPreprocessing, pProperties)) {
        pimpl->mIOSystem = old;
        return nullptr;
    }

    pimpl->blob = blobio->GetBlobChain();
    pimpl->mIOSystem = old;

    return pimpl->blob;
}

// ------------------------------------------------------------------------------------------------
/*
pScene			输入场景
pFormatId		输出文件格式如“obj”
pPath			路径地址
pPreprocessing	预处理的操作集
pProperties		输出属性设置
*/
aiReturn Exporter::Export( const aiScene* pScene, const char* pFormatId, const char* pPath,
        unsigned int pPreprocessing, const ExportProperties* pProperties) {
    ASSIMP_BEGIN_EXCEPTION_REGION();
	ai_assert(nullptr != pimpl);//ExporterPimpl *pimpl; Exporter.hpp 334
    // when they create scenes from scratch, users will likely create them not in verbose
    // format. They will likely not be aware that there is a flag in the scene to indicate
    // this, however. To avoid surprises and bug reports, we check for duplicates in
    // meshes upfront.
	//创建场景时，用户很可能不会创建详细格式（verbose format）。然而，scene中有一个标志表示这一点。为了避免意外和错误报告，我们在网格的前面检查重复。
    const bool is_verbose_format = !(pScene->mFlags & AI_SCENE_FLAGS_NON_VERBOSE_FORMAT) || MakeVerboseFormatProcess::IsVerboseFormat(pScene);

    pimpl->mProgressHandler->UpdateFileWrite(0, 4);//第0步：开始输出处理，筛选输出格式

    pimpl->mError = "";
    for (size_t i = 0; i < pimpl->mExporters.size(); ++i) {	//mExporters记录了所有导出格式的信息，此处for来找到目标格式，如“obj”
        const Exporter::ExportFormatEntry& exp = pimpl->mExporters[i]; //exp是引用
        if (!strcmp(exp.mDescription.id,pFormatId)) { //当导出类型pFormatId==exp.mDescription.id，找到目标格式
            try {
                // Always create a full copy of the scene. 
                aiScene* scenecopy_tmp = nullptr;//复制整个pscene
                SceneCombiner::CopyScene(&scenecopy_tmp,pScene);

                pimpl->mProgressHandler->UpdateFileWrite(1, 4);//第1步：获取输出函数掩码汇总

                std::unique_ptr<aiScene> scenecopy(scenecopy_tmp);
                const ScenePrivateData* const priv = ScenePriv(pScene);//调用ScenePrivate.h中函数，priv获得其中的私有数据

                // steps that are not idempotent, i.e. we might need to run them again, usually to get back to the
                // original state before the step was applied first. When checking which steps we don't need
                // to run, those are excluded.
				//非幂等的步骤（幂等：任意多次执行所产生的影响均与一次执行的影响相同）
				//例如，我们可能需要再次运行它们，通常是为了在第一次应用该步骤之前回到原始状态。在检查哪些步骤不需要运行时，这些步骤被排除在外。
                const unsigned int nonIdempotentSteps = aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_MakeLeftHanded;//获取非幂等的操作
				//上述的状态在postprocess.h中，分别表示法向、uv方向、左手系等状态。

                // Erase all pp steps that were already applied to this scene
				//★ 删除已经应用到这个场景的所有 后处理 步骤
				const unsigned int pp = (exp.mEnforcePP | pPreprocessing) & ~(priv && !priv->mIsCopy ? (priv->mPPStepsApplied & ~nonIdempotentSteps) : 0u);
				//pp（post processing）后处理，这里是删除掉场景所带的后处理步骤之后的后处理总和
				/*			详细分析
				//（mEnforcePP记录执行的后处理操作 | 输入的预处理操作pPreprocessing）即，统计已经进行的后处理操作（包括输出类型本身携带的后处理，及输入的后处理）
				//（priv && !priv->mIsCopy）私有数据存在，且未进行修改，则
				//(priv->mPPStepsApplied & ~nonIdempotentSteps) 提取priv中对scene的后处理中除了nonIdempotentSteps所述的三个操作的其他操作
				//0u 若对私有数据修改过或不存在，则置零。
				//pp即 (exp.mEnforcePP | pPreprocessing)提取非私有数据priv->mPPStepsApplied中未出现过的操作，但是“三个操作”出现过也可以在提取一次
				*/

                // If no extra post-processing was specified, and we obtained this scene from an Assimp importer, apply the reverse steps automatically.
				//如果没有指定额外的后处理，并且我们从Assimp导入器获得了这个场景，则自动应用反向步骤。
                // TODO: either drop this, or document it. Otherwise it is just a bad surprise.
                //if (!pPreprocessing && priv) {
                //  pp |= (nonIdempotentSteps & priv->mPPStepsApplied);
                //}

                // If the input scene is not in verbose format, but there is at least post-processing step that relies on it,
                // we need to run the MakeVerboseFormat step first.
				//非verbose format但是有后处理要依赖，我们则先运行MakeVerboseFormat
                bool must_join_again = false;
                if (!is_verbose_format) {
                    bool verbosify = false;
                    for( unsigned int a = 0; a < pimpl->mPostProcessingSteps.size(); a++)  //逐个处理后处理步骤
					{
                        BaseProcess* const p = pimpl->mPostProcessingSteps[a];//p获取基础操作 // BaseProcess.h

                        if (p->IsActive(pp) && p->RequireVerboseFormat()) {  //该步骤需要verbose格式
                            verbosify = true;
                            break;
                        }
                    }

                    if (verbosify || (exp.mEnforcePP & aiProcess_JoinIdenticalVertices)) {//如果需要verbose格式，或mEnforcePP中有aiProcess_JoinIdenticalVertices
                        ASSIMP_LOG_DEBUG("export: Scene data not in verbose format, applying MakeVerboseFormat step first");

                        MakeVerboseFormatProcess proc;
                        proc.Execute(scenecopy.get());

                        if(!(exp.mEnforcePP & aiProcess_JoinIdenticalVertices)) {//不是因为mEnforcePP中有aiProcess_JoinIdenticalVertices
                            must_join_again = true;
                        }
                    }
                }

                pimpl->mProgressHandler->UpdateFileWrite(2, 4);//第2步：进行后处理

                if (pp) {
                    // the three 'conversion' steps need to be executed first because all other steps rely on the standard data layout
					//首先需要执行三个“转换”步骤，因为所有其他步骤都依赖于标准数据布局
                    {
                        FlipWindingOrderProcess step;//法向量方向
                        if (step.IsActive(pp)) {
                            step.Execute(scenecopy.get());
                        }
                    }

                    {
                        FlipUVsProcess step;//uv贴图方向
                        if (step.IsActive(pp)) {
                            step.Execute(scenecopy.get());
                        }
                    }

                    {
                        MakeLeftHandedProcess step;//左手系
                        if (step.IsActive(pp)) {
                            step.Execute(scenecopy.get());
                        }
                    }

                    bool exportPointCloud(false);
                    if (nullptr != pProperties) {  //引用的时候没有设置输出属性值，pProperties=null
                        exportPointCloud = pProperties->GetPropertyBool(AI_CONFIG_EXPORT_POINT_CLOUDS);
                    }
					
                    // dispatch other processes //其他后处理步骤
                    for( unsigned int a = 0; a < pimpl->mPostProcessingSteps.size(); a++) 
					{
                        BaseProcess* const p = pimpl->mPostProcessingSteps[a];

                        if (p->IsActive(pp) && !dynamic_cast<FlipUVsProcess*>(p) && !dynamic_cast<FlipWindingOrderProcess*>(p) && !dynamic_cast<MakeLeftHandedProcess*>(p)) 
						{
                            if (dynamic_cast<PretransformVertices*>(p) && exportPointCloud) {
                                continue;
                            }
                            p->Execute(scenecopy.get());
                        }
                    }
                    ScenePrivateData* const privOut = ScenePriv(scenecopy.get());  //ScenePrivate.h中。
                    ai_assert(nullptr != privOut);

                    privOut->mPPStepsApplied |= pp;  //将后处理的操作pp添加到privOut中
                }

                pimpl->mProgressHandler->UpdateFileWrite(3, 4);//第3步：根据格式导出。

                if(must_join_again) {
                    JoinVerticesProcess proc;
                    proc.Execute(scenecopy.get());
                }

                ExportProperties emptyProperties;  // Never pass nullptr ExportProperties so Exporters don't have to worry.//输出值处理类实例化，这里是当调用的输出属性值为空时调用
                ExportProperties* pProp = pProperties ? (ExportProperties*)pProperties : &emptyProperties;//调用函数的时候未设置，则为空
        		pProp->SetPropertyBool("bJoinIdenticalVertices", pp & aiProcess_JoinIdenticalVertices);//添加输出属性“bJoinIdenticalVertices”
				//【函数指针】此处调用“obj”函数//exp381行定义；scenecopy 386行;//exp初始化时该函数定义。
                exp.mExportFunction(pPath,pimpl->mIOSystem.get(),scenecopy.get(), pProp);//★函数指针，指向所引用类型的函数，如obj调用函数ExportSceneObj。

                pimpl->mProgressHandler->UpdateFileWrite(4, 4);
            } catch (DeadlyExportError& err) {  //err初始化为0
                pimpl->mError = err.what();//错误类型
                return AI_FAILURE;  //aiReturn_FAILURE
            }
            return AI_SUCCESS;//当可以找到我们要导出的类型且无误直接返回。
        }
    }

    pimpl->mError = std::string("Found no exporter to handle this file format: ") + pFormatId;//找不到要导出的类型
    ASSIMP_END_EXCEPTION_REGION(aiReturn);

    return AI_FAILURE;
}

// ------------------------------------------------------------------------------------------------
const char* Exporter::GetErrorString() const {
	ai_assert(nullptr != pimpl);
    return pimpl->mError.c_str();
}

// ------------------------------------------------------------------------------------------------
void Exporter::FreeBlob() {
	ai_assert(nullptr != pimpl);
    delete pimpl->blob;
    pimpl->blob = nullptr;

    pimpl->mError = "";
}

// ------------------------------------------------------------------------------------------------
const aiExportDataBlob* Exporter::GetBlob() const {
	ai_assert(nullptr != pimpl);
	return pimpl->blob;
}

// ------------------------------------------------------------------------------------------------
const aiExportDataBlob* Exporter::GetOrphanedBlob() const {
	ai_assert(nullptr != pimpl);
	const aiExportDataBlob *tmp = pimpl->blob;
    pimpl->blob = nullptr;
    return tmp;
}

// ------------------------------------------------------------------------------------------------
size_t Exporter::GetExportFormatCount() const {
	ai_assert(nullptr != pimpl);
    return pimpl->mExporters.size();
}

// ------------------------------------------------------------------------------------------------
const aiExportFormatDesc* Exporter::GetExportFormatDescription( size_t index ) const {
	ai_assert(nullptr != pimpl);
	if (index >= GetExportFormatCount()) {
        return nullptr;
    }

    // Return from static storage if the requested index is built-in.
	if (index < pimpl->mExporters.size()) {
		return &pimpl->mExporters[index].mDescription;
    }

    return &pimpl->mExporters[index].mDescription;
}

// ------------------------------------------------------------------------------------------------
aiReturn Exporter::RegisterExporter(const ExportFormatEntry& desc) {
	ai_assert(nullptr != pimpl);
	for (const ExportFormatEntry &e : pimpl->mExporters) {
        if (!strcmp(e.mDescription.id,desc.mDescription.id)) {
            return aiReturn_FAILURE;
        }
    }

    pimpl->mExporters.push_back(desc);
    return aiReturn_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
void Exporter::UnregisterExporter(const char* id) {
	ai_assert(nullptr != pimpl);
	for (std::vector<ExportFormatEntry>::iterator it = pimpl->mExporters.begin();
            it != pimpl->mExporters.end(); ++it) {
        if (!strcmp((*it).mDescription.id,id)) {
            pimpl->mExporters.erase(it);
            break;
        }
    }
}


/*
					【以下为类ExportProperties的函数实现（原类在 Exporter.hpp）】
*/
// ---------------------------------------构造---------------------------------------------------------
ExportProperties::ExportProperties() {
    // empty
}

// ---------------------------------------拷贝构造---------------------------------------------------------
ExportProperties::ExportProperties(const ExportProperties &other)
: mIntProperties(other.mIntProperties)
, mFloatProperties(other.mFloatProperties)
, mStringProperties(other.mStringProperties)
, mMatrixProperties(other.mMatrixProperties)
, mCallbackProperties(other.mCallbackProperties){
    // empty
}

bool ExportProperties::SetPropertyCallback(const char *szName, const std::function<void *(void *)> &f) {
    return SetGenericProperty<std::function<void *(void *)>>(mCallbackProperties, szName, f);
}

std::function<void *(void *)> ExportProperties::GetPropertyCallback(const char *szName) const {
    return GetGenericProperty<std::function<void *(void *)>>(mCallbackProperties, szName, 0);
}

bool ExportProperties::HasPropertyCallback(const char *szName) const {
    return HasGenericProperty<std::function<void *(void *)>>(mCallbackProperties, szName);
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool ExportProperties::SetPropertyInteger(const char* szName, int iValue) {
    return SetGenericProperty<int>(mIntProperties, szName,iValue);
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool ExportProperties::SetPropertyFloat(const char* szName, ai_real iValue) {
    return SetGenericProperty<ai_real>(mFloatProperties, szName,iValue);
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool ExportProperties::SetPropertyString(const char* szName, const std::string& value) {
    return SetGenericProperty<std::string>(mStringProperties, szName,value);
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool ExportProperties::SetPropertyMatrix(const char* szName, const aiMatrix4x4& value) {
    return SetGenericProperty<aiMatrix4x4>(mMatrixProperties, szName,value);
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
int ExportProperties::GetPropertyInteger(const char* szName, int iErrorReturn /*= 0xffffffff*/) const {
    return GetGenericProperty<int>(mIntProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
ai_real ExportProperties::GetPropertyFloat(const char* szName, ai_real iErrorReturn /*= 10e10*/) const {
    return GetGenericProperty<ai_real>(mFloatProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
const std::string ExportProperties::GetPropertyString(const char* szName,
        const std::string& iErrorReturn /*= ""*/) const {
    return GetGenericProperty<std::string>(mStringProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Has a configuration property
const aiMatrix4x4 ExportProperties::GetPropertyMatrix(const char* szName,
        const aiMatrix4x4& iErrorReturn /*= aiMatrix4x4()*/) const {
    return GetGenericProperty<aiMatrix4x4>(mMatrixProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Has a configuration property
bool ExportProperties::HasPropertyInteger(const char* szName) const {
    return HasGenericProperty<int>(mIntProperties, szName);
}

// ------------------------------------------------------------------------------------------------
// Has a configuration property
bool ExportProperties::HasPropertyBool(const char* szName) const {
    return HasGenericProperty<int>(mIntProperties, szName);
}

// ------------------------------------------------------------------------------------------------
// Has a configuration property
bool ExportProperties::HasPropertyFloat(const char* szName) const {
    return HasGenericProperty<ai_real>(mFloatProperties, szName);
}

// ------------------------------------------------------------------------------------------------
// Has a configuration property
bool ExportProperties::HasPropertyString(const char* szName) const {
    return HasGenericProperty<std::string>(mStringProperties, szName);
}

// ------------------------------------------------------------------------------------------------
// Has a configuration property
bool ExportProperties::HasPropertyMatrix(const char* szName) const {
    return HasGenericProperty<aiMatrix4x4>(mMatrixProperties, szName);
}


#endif // !ASSIMP_BUILD_NO_EXPORT

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

/** @file scene.h
 *  @brief Defines the data structures in which the imported scene is returned.
 */
#pragma once
#ifndef AI_SCENE_H_INC
#define AI_SCENE_H_INC

#ifdef __GNUC__
#   pragma GCC system_header
#endif

#include <assimp/types.h>
#include <assimp/texture.h>
#include <assimp/mesh.h>
#include <assimp/light.h>
#include <assimp/camera.h>
#include <assimp/material.h>
#include <assimp/anim.h>
#include <assimp/metadata.h>

#ifdef __cplusplus
#  include <cstdlib>
extern "C" {
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

// -------------------------------------------------------------------------------
/** 
 * A node in the imported hierarchy.
 *
 * Each node has name, a parent node (except for the root node),
 * a transformation relative to its parent and possibly several child nodes.
 * Simple file formats don't support hierarchical structures - for these formats
 * the imported scene does consist of only a single root node without children.
 */
// -------------------------------------------------------------------------------
struct ASSIMP_API aiNode
{
    /** The name of the node.//节点名称
     *
     * The name might be empty (length of zero) but all nodes which
     * need to be referenced by either bones or animations are named.
     * Multiple nodes may have the same name, except for nodes which are referenced
     * by bones (see #aiBone and #aiMesh::mBones). Their names *must* be unique.
     *
     * Cameras and lights reference a specific node by name - if there
     * are multiple nodes with this name, they are assigned to each of them.
     * <br>
     * There are no limitations with regard to the characters contained in
     * the name string as it is usually taken directly from the source file.
     *
     * Implementations should be able to handle tokens such as whitespace, tabs,
     * line feeds, quotation marks, ampersands etc.
     *
     * Sometimes assimp introduces new nodes not present in the source file
     * into the hierarchy (usually out of necessity because sometimes the
     * source hierarchy format is simply not compatible). Their names are
     * surrounded by @verbatim <> @endverbatim e.g.
     *  @verbatim<DummyRootNode> @endverbatim.
     */
    C_STRUCT aiString mName; //名

    /** The transformation relative to the node's parent. */
    C_STRUCT aiMatrix4x4 mTransformation;  //变换矩阵

    /** Parent node. nullptr if this node is the root node. */
    C_STRUCT aiNode* mParent;  //父节点

    /** The number of child nodes of this node. */
    unsigned int mNumChildren;  //子节点数量

    /** The child nodes of this node. nullptr if mNumChildren is 0. */
    C_STRUCT aiNode** mChildren;  //子节点

    /** The number of meshes of this node. */
    unsigned int mNumMeshes;  //mesh数量

    /** The meshes of this node. Each entry is an index into the
      * mesh list of the #aiScene.
	  //子节点记录的是index
      */
    unsigned int* mMeshes;  //节点的mesh

    /** Metadata associated with this node or nullptr if there is no metadata.
      *  Whether any metadata is generated depends on the source file format. See the
      * @link importer_notes @endlink page for more information on every source file
      * format. Importers that don't document any metadata don't write any.
      */
    C_STRUCT aiMetadata* mMetaData;  //与节点相关的meta data

#ifdef __cplusplus  //c++使用
    /** Constructor */
    aiNode();

    /** Construction from a specific name */
    explicit aiNode(const std::string& name);

    /** Destructor */
    ~aiNode();

    /** Searches for a node with a specific name, beginning at this
     *  nodes. Normally you will call this method on the root node
     *  of the scene.
     *
     *  @param name Name to search for
     *  @return nullptr or a valid Node if the search was successful.
     */
    inline 
    const aiNode* FindNode(const aiString& name) const {
        return FindNode(name.data);
    }

    inline 
    aiNode* FindNode(const aiString& name) {
        return FindNode(name.data);
    }

    const aiNode* FindNode(const char* name) const;

    aiNode* FindNode(const char* name);

    /**
     * @brief   Will add new children.
     * @param   numChildren  Number of children to add.
     * @param   children     The array with pointers showing to the children.
     */
    void addChildren(unsigned int numChildren, aiNode **children);
#endif // __cplusplus
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

// -------------------------------------------------------------------------------
/**
 * Specifies that the scene data structure that was imported is not complete.
 * This flag bypasses some internal validations and allows the import
 * of animation skeletons, material libraries or camera animation paths
 * using Assimp. Most applications won't support such data.
 //指定导入的场景数据结构不完整。该标志绕过了一些内部验证，并允许使用Assimp导入动画骨架、材料库或摄像机动画路径。大多数applications不支持这种数据。
 */
#define AI_SCENE_FLAGS_INCOMPLETE   0x1

/**
 * This flag is set by the validation postprocess-step (aiPostProcess_ValidateDS)
 * if the validation is successful. In a validated scene you can be sure that
 * any cross references in the data structure (e.g. vertex indices) are valid.
 //如果验证成功，则验证后处理步骤(aiPostProcess_ValidateDS)设置此标志。在一个有效的场景中，你可以确保数据结构中的任何交叉引用(例如顶点索引)都是有效的。
 */
#define AI_SCENE_FLAGS_VALIDATED    0x2

/**
 * This flag is set by the validation postprocess-step (aiPostProcess_ValidateDS)
 * if the validation is successful but some issues have been found.
 * This can for example mean that a texture that does not exist is referenced
 * by a material or that the bone weights for a vertex don't sum to 1.0 ... .
 * In most cases you should still be able to use the import. This flag could
 * be useful for applications which don't capture Assimp's log output.
 //如果验证成功，则验证后处理步骤(aiPostProcess_ValidateDS)设置此标志，但是有些警告。
 //例如一个不存在的纹理被一个材料引用，或者一个顶点的骨重量之和不是1.0 ... .在大多数情况下，您仍然可以使用导入。这个标志对于不捕获Assimp日志输出的应用程序可能很有用。
 */
#define AI_SCENE_FLAGS_VALIDATION_WARNING   0x4

/**
 * This flag is currently only set by the aiProcess_JoinIdenticalVertices step.
 * It indicates that the vertices of the output meshes aren't in the internal
 * verbose format anymore. In the verbose format all vertices are unique,
 * no vertex is ever referenced by more than one face.
 //只发生在aiProcess_JoinIdenticalVertices步骤，它表明输出网格的顶点不再是内部详细格式（verbose format）。在详细格式中，所有顶点都是唯一的，没有顶点会被多个面引用。
 */
#define AI_SCENE_FLAGS_NON_VERBOSE_FORMAT   0x8  //mesh的vertices不是详细格式

 /**
 * Denotes pure height-map terrain data. Pure terrains usually consist of quads,
 * sometimes triangles, in a regular grid. The x,y coordinates of all vertex
 * positions refer to the x,y coordinates on the terrain height map, the z-axis
 * stores the elevation at a specific point.
 //表示纯高度图地形数据。纯地形通常由规则网格中的四边形组成，有时是三角形。所有顶点位置的x、y坐标是指地形高度图上的x、y坐标，z轴存储特定点的高程。
 *
 * TER (Terragen) and HMP (3D Game Studio) are height map formats.
 * @note Assimp is probably not the best choice for loading *huge* terrains -
 * fully triangulated data takes extremely much free store and should be avoided
 * as long as possible (typically you'll do the triangulation when you actually
 * need to render it).
 */
#define AI_SCENE_FLAGS_TERRAIN 0x10

 /**
 * Specifies that the scene data can be shared between structures. For example:
 * one vertex in few faces. \ref AI_SCENE_FLAGS_NON_VERBOSE_FORMAT can not be
 * used for this because \ref AI_SCENE_FLAGS_NON_VERBOSE_FORMAT has internal
 * meaning about postprocessing steps.
 //指定场景数据可以在结构之间共享。例如:一个顶点在几个面。\ref AI_SCENE_FLAGS_NON_VERBOSE_FORMAT不能用于此，因为它有关于后处理步骤的内部含义。
 */
#define AI_SCENE_FLAGS_ALLOW_SHARED			0x20

// -------------------------------------------------------------------------------
/** The root structure of the imported data.
 *
 *  Everything that was imported from the given file can be accessed from here.
 *  Objects of this class are generally maintained and owned by Assimp, not
 *  by the caller. You shouldn't want to instance it, nor should you ever try to
 *  delete a given scene on your own.
 //从给定文件导入的所有内容都可以从这里访问。该类的对象通常由Assimp维护和拥有，而不是由调用者。你不应该想要实例化它，也不应该试图自己删除一个给定的场景。
 */
// -------------------------------------------------------------------------------
struct aiScene
{
    /** Any combination of the AI_SCENE_FLAGS_XXX flags. By default
    * this value is 0, no flags are set. Most applications will
    * want to reject all scenes with the AI_SCENE_FLAGS_INCOMPLETE
    * bit set.
    */
    unsigned int mFlags; //AI_SCENE_FLAGS_XXX 的各种flag结合

    /** The root node of the hierarchy.
    *
    * There will always be at least the root node if the import
    * was successful (and no special flags have been set).
    * Presence of further nodes depends on the format and content
    * of the imported file.
	//如果导入成功(并且没有设置特殊标志)，则至少会有根节点。其他节点的存在取决于导入文件的格式和内容。
    */
    C_STRUCT aiNode* mRootNode;//结构的根节点

    /** The number of meshes in the scene. */
    unsigned int mNumMeshes;//mesh数量

    /** The array of meshes.
    *
    * Use the indices given in the aiNode structure to access
    * this array. The array is mNumMeshes in size. If the
    * AI_SCENE_FLAGS_INCOMPLETE flag is not set there will always
    * be at least ONE material.
    */
    C_STRUCT aiMesh** mMeshes;//mesh的array

    /** The number of materials in the scene. */
    unsigned int mNumMaterials;//material数量

    /** The array of materials.
    *
    * Use the index given in each aiMesh structure to access this
    * array. The array is mNumMaterials in size. If the
    * AI_SCENE_FLAGS_INCOMPLETE flag is not set there will always
    * be at least ONE material.
    */
    C_STRUCT aiMaterial** mMaterials;//material的array

    /** The number of animations in the scene. */
    unsigned int mNumAnimations;//animation数量

    /** The array of animations.
    *
    * All animations imported from the given file are listed here.
    * The array is mNumAnimations in size.
    */
    C_STRUCT aiAnimation** mAnimations;//animation的array

    /** The number of textures embedded into the file */
    unsigned int mNumTextures;//texture的数量

    /** The array of embedded textures.
    *
    * Not many file formats embed their textures into the file.
    * An example is Quake's MDL format (which is also used by
    * some GameStudio versions)
    */
    C_STRUCT aiTexture** mTextures;//textrue的array

    /** The number of light sources in the scene. Light sources
    * are fully optional, in most cases this attribute will be 0
	//光源数量
        */
    unsigned int mNumLights;//light数量

    /** The array of light sources.
    *
    * All light sources imported from the given file are
    * listed here. The array is mNumLights in size.
    */
    C_STRUCT aiLight** mLights;//light的array

    /** The number of cameras in the scene. Cameras
    * are fully optional, in most cases this attribute will be 0
        */
    unsigned int mNumCameras;//camera数量

    /** The array of cameras.
    *
    * All cameras imported from the given file are listed here.
    * The array is mNumCameras in size. The first camera in the
    * array (if existing) is the default camera view into
    * the scene.
    */
    C_STRUCT aiCamera** mCameras;//camera的array

    /**
     *  @brief  The global metadata assigned to the scene itself.
     *
     *  This data contains global metadata which belongs to the scene like
     *  unit-conversions, versions, vendors or other model-specific data. This
     *  can be used to store format-specific metadata as well.
	 //metadata（元数据），包括场景的单位转换、版本、提供者等其他模型特殊数据，也可用作储存格式特殊数据
     */
    C_STRUCT aiMetadata* mMetaData;

    /** The name of the scene itself.
     */
    C_STRUCT aiString mName;//scene名字

#ifdef __cplusplus  //c++

    //! Default constructor - set everything to 0/nullptr
    ASSIMP_API aiScene();

    //! Destructor
    ASSIMP_API ~aiScene();

    //! Check whether the scene contains meshes
    //! Unless no special scene flags are set this will always be true.
    inline bool HasMeshes() const {  //判断是否有mesh
        return mMeshes != nullptr && mNumMeshes > 0; 
    }

    //! Check whether the scene contains materials
    //! Unless no special scene flags are set this will always be true.
    inline bool HasMaterials() const {   //判断是否包含materials
        return mMaterials != nullptr && mNumMaterials > 0; 
    }

    //! Check whether the scene contains lights
    inline bool HasLights() const { 
        return mLights != nullptr && mNumLights > 0; 
    }

    //! Check whether the scene contains textures
    inline bool HasTextures() const {
        return mTextures != nullptr && mNumTextures > 0; 
    }

    //! Check whether the scene contains cameras
    inline bool HasCameras() const {
        return mCameras != nullptr && mNumCameras > 0; 
    }

    //! Check whether the scene contains animations
    inline bool HasAnimations() const { 
        return mAnimations != nullptr && mNumAnimations > 0; 
    }

    //! Returns a short filename from a full path//从路径中读取文件名
    static const char* GetShortFilename(const char* filename) {  
        const char* lastSlash = strrchr(filename, '/');
        if (lastSlash == nullptr) {
            lastSlash = strrchr(filename, '\\');
        }
        const char* shortFilename = lastSlash != nullptr ? lastSlash + 1 : filename;
        return shortFilename;
    }

    //! Returns an embedded texture//返回插入材质
    const aiTexture* GetEmbeddedTexture(const char* filename) const {
        // lookup using texture ID (if referenced like: "*1", "*2", etc.)
        if ('*' == *filename) {
            int index = std::atoi(filename + 1);
            if (0 > index || mNumTextures <= static_cast<unsigned>(index))
                return nullptr;
            return mTextures[index];
        }
        // lookup using filename
        const char* shortFilename = GetShortFilename(filename);
        for (unsigned int i = 0; i < mNumTextures; i++) {
            const char* shortTextureFilename = GetShortFilename(mTextures[i]->mFilename.C_Str());
            if (strcmp(shortTextureFilename, shortFilename) == 0) {
                return mTextures[i];
            }
        }
        return nullptr;
    }
#endif // __cplusplus

    /**  Internal data, do not touch */
#ifdef __cplusplus
    void* mPrivate;
#else
    char* mPrivate;
#endif

};

#ifdef __cplusplus
} //! namespace Assimp
#endif

#endif // AI_SCENE_H_INC

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

/** @file ProgressHandler.hpp
 *  @brief Abstract base class 'ProgressHandler'.
 */
#pragma once
#ifndef AI_PROGRESSHANDLER_H_INC
#define AI_PROGRESSHANDLER_H_INC

#ifdef __GNUC__
#   pragma GCC system_header
#endif

#include <assimp/types.h>

namespace Assimp {

// ------------------------------------------------------------------------------------
/** @brief CPP-API: Abstract interface for custom progress report receivers.
//自定义报告接收器的抽象接口
 *
 *  Each #Importer instance maintains its own #ProgressHandler. The default
 *  implementation provided by Assimp doesn't do anything at all. */
 //每 个#Importer 实例都维护自己的 #ProgressHandler。 Assimp提供的默认实现根本不做任何事情。
class ASSIMP_API ProgressHandler
#ifndef SWIG
    : public Intern::AllocateFromAssimpHeap
#endif
{
protected:
    /// @brief  Default constructor
    ProgressHandler () AI_NO_EXCEPT {
        // empty
    }

public:
    /// @brief  Virtual destructor.
    virtual ~ProgressHandler () {
    }

    // -------------------------------------------------------------------
    /** @brief Progress callback.		//回调
     *  @param #percentage An estimate of the current loading progress,		//加载进度
     *    in percent. Or -1.f if such an estimate is not available.
     *
     *  There are restriction on what you may do from within your
     *  implementation of this method: no exceptions may be thrown and no
     *  non-const #Importer methods may be called. It is
     *  not generally possible to predict the number of callbacks
     *  fired during a single import.
     *
     *  @return Return false to abort loading at the next possible			//返回false以在下一次可能的情况下中止加载
     *   occasion (loaders and Assimp are generally allowed to perform
     *   all needed cleanup tasks prior to returning control to the
     *   caller). If the loading is aborted, #Importer::ReadFile()
     *   returns always nullptr.
     *   */
    virtual bool Update(float percentage = -1.f) = 0;

    // -------------------------------------------------------------------
    /** @brief Progress callback for file loading steps
     *  @param numberOfSteps The number of total post-processing
     *   steps
     *  @param currentStep The index of the current post-processing
     *   step that will run, or equal to numberOfSteps if all of
     *   them has finished. This number is always strictly monotone
     *   increasing, although not necessarily linearly.
     *
     *  @note This is currently only used at the start and the end
     *   of the file parsing.
     *   */
    virtual void UpdateFileRead(int currentStep /*= 0*/, int numberOfSteps /*= 0*/) {
        float f = numberOfSteps ? currentStep / (float)numberOfSteps : 1.0f;
        Update( f * 0.5f );
    }

    // -------------------------------------------------------------------
    /** @brief Progress callback for post-processing steps
     *  @param numberOfSteps The number of total post-processing
     *   steps
     *  @param currentStep The index of the current post-processing
     *   step that will run, or equal to numberOfSteps if all of
     *   them has finished. This number is always strictly monotone
     *   increasing, although not necessarily linearly.
     *   */
    virtual void UpdatePostProcess(int currentStep /*= 0*/, int numberOfSteps /*= 0*/) {
        float f = numberOfSteps ? currentStep / (float)numberOfSteps : 1.0f;
        Update( f * 0.5f + 0.5f );
    }


    // -------------------------------------------------------------------
    /** @brief Progress callback for export steps.
     *  @param #numberOfSteps The number of total processing steps				//numberOfSteps 总步骤的数量
     *  @param #currentStep The index of the current post-processing			//当前步骤
     *   step that will run, or equal to numberOfSteps if all of
     *   them has finished. This number is always strictly monotone
     *   increasing, although not necessarily linearly.
	 //currentStep 当前要处理的步骤的索引，这个数字总是严格单调递增的，尽管不一定是线性的。
     *   */
    virtual void UpdateFileWrite(int currentStep /*= 0*/, int numberOfSteps /*= 0*/) {	//进度说明
        float f = numberOfSteps ? currentStep / (float)numberOfSteps : 1.0f;
        Update(f * 0.5f);	//98行
    }
}; // !class ProgressHandler

// ------------------------------------------------------------------------------------

} // Namespace Assimp

#endif // AI_PROGRESSHANDLER_H_INC

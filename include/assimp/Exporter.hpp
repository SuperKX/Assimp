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

/** @file  Exporter.hpp
*  @brief Defines the CPP-API for the Assimp export interface
*/
#pragma once
#ifndef AI_EXPORT_HPP_INC
#define AI_EXPORT_HPP_INC

#ifdef __GNUC__
#pragma GCC system_header
#endif

#ifndef ASSIMP_BUILD_NO_EXPORT

#include "cexport.h"
#include <map>
#include <functional>

namespace Assimp {

class ExporterPimpl;//��exporter.cpp�ж���
class IOSystem;  //��IOSystem.hpp��
class ProgressHandler;//ProgressHandler.hpp

// ----------------------------------------------------------------------------------
/** CPP-API: The Exporter class forms an C++ interface to the export functionality
 * of the Open Asset Import Library. Note that the export interface is available
 * only if Assimp has been built with ASSIMP_BUILD_NO_EXPORT not defined.
 //Exporter��ΪAssimp���ṩ��һ���������ܵ�c++�ӿڡ�ֻ���ڹ���Assimpʱ��ASSIMP_BUILD_NO_EXPORTû�ж���ʱ���ſ���
 *
 * The interface is modeled after the importer interface and mostly
 * symmetric. The same rules for threading etc. apply.
 //��importer�����ǶԳƵģ����̵߳Ĺ�����ͬ��
 *
 * In a nutshell, there are two export interfaces: #Export, which writes the
 * output file(s) either to the regular file system or to a user-supplied
 * #IOSystem, and #ExportToBlob which returns a linked list of memory
 * buffers (blob), each referring to one output file (in most cases
 * there will be only one output file of course, but this extra complexity is
 * needed since Assimp aims at supporting a wide range of file formats).
 //ͨ����������ӿ�:
 //1��#Export,������ļ�д���� �����ļ�ϵͳ�����û��ṩ��#IOSystem,
 //2��#ExportToBlob ����һ���ڴ滺����(blob)������,ÿ��ָһ������ļ�(�ڴ��������»���ֻ��һ������ļ�,������Ҫ����ĸ�������ΪAssimpּ��֧�ֶ����ļ���ʽ)��
 *
 * #ExportToBlob is especially useful if you intend to work
 * with the data in-memory.
 //#ExportToBlob���ڴ��ڹ���ʱ�Ǻ����õġ�
*/
class ASSIMP_API ExportProperties;//331�и�������

class ASSIMP_API Exporter {
public:
    /** Function pointer type of a Export worker function */  //���嵼������������worker function�����ĺ���ָ������
	//����ʽ��  typedef �������� (*������)(������)
	//������һ�ֺ���ָ�룬����ָ�����ָ�����⺯����ֻҪ��������Ϊ��(������)��������Ϊ���������� �����ɡ�
    typedef void (*fpExportFunc)(const char *, IOSystem *, const aiScene *, const ExportProperties *);

    /** Internal description of an Assimp export format option */  //������ʽѡ����ڲ�����
    struct ExportFormatEntry {
		//��Ա�������ʽ���������õ����������������
        //�� Public description structure to be returned by aiGetExportFormatDescription()
        aiExportFormatDesc mDescription; //�����ʽ����

        //�� Worker function to do the actual exporting//ʵ�ʵ����ĸ�������
        fpExportFunc mExportFunction;//������ָ�롿ʵ������һ��ָ�루���캯�������г�ʼ����

        //�� Post-processing steps to be executed PRIOR to invoking mExportFunction//�����豻ִ�����ڵ���mExportFunction
        unsigned int mEnforcePP;//��¼ִ�еĺ������

        // Constructor to fill all entries//���캯��
        ExportFormatEntry(const char *pId, const char *pDesc, const char *pExtension, fpExportFunc pFunction, unsigned int pEnforcePP = 0u) {
            mDescription.id = pId;  //"obj"
            mDescription.description = pDesc;  //"Wavefront OBJ format"
            mDescription.fileExtension = pExtension;  //"obj"
            mExportFunction = pFunction;//����ָ��ָ����һ������pFunction
            mEnforcePP = pEnforcePP;
        }

        ExportFormatEntry() :
			mExportFunction(),
			mEnforcePP() {
            mDescription.id = nullptr;
            mDescription.description = nullptr;
            mDescription.fileExtension = nullptr;
        }
    };

    /**
     *  @brief  The class constructor.//����
     */
    Exporter();

    /**
    *  @brief  The class destructor.//����
    */
    ~Exporter();


	/*                                ��������                            */	
    // -------------------------------------------------------------------
    /** Supplies a custom IO handler to the exporter to use to open and
     * access files.
     *
     * If you need #Export to use custom IO logic to access the files,
     * you need to supply a custom implementation of IOSystem and
     * IOFile to the exporter.
     *
     * #Exporter takes ownership of the object and will destroy it
     * afterwards. The previously assigned handler will be deleted.
     * Pass NULL to take again ownership of your IOSystem and reset Assimp
     * to use its default implementation, which uses plain file IO.
     *
     * @param pIOHandler The IO handler to be used in all file accesses
     *   of the Importer. */
    void SetIOHandler(IOSystem *pIOHandler);//�򵼳������ṩһ���Զ���IO����������ڴ򿪺ͷ����ļ���

    // -------------------------------------------------------------------
    /** Retrieves the IO handler that is currently set.
     * You can use #IsDefaultIOHandler() to check whether the returned
     * interface is the default IO handler provided by ASSIMP. The default
     * handler is active as long the application doesn't supply its own
     * custom IO handler via #SetIOHandler().
     * @return A valid IOSystem interface, never NULL. */
    IOSystem *GetIOHandler() const;

    // -------------------------------------------------------------------
    /** Checks whether a default IO handler is active
     * A default handler is active as long the application doesn't
     * supply its own custom IO handler via #SetIOHandler().
     * @return true by default */
    bool IsDefaultIOHandler() const;  //������ǰ���õ�IO�������

    // -------------------------------------------------------------------
    /** Supplies a custom progress handler to the exporter. This
     *  interface exposes an #Update() callback, which is called
     *  more or less periodically (please don't sue us if it
     *  isn't as periodically as you'd like it to have ...).
     *  This can be used to implement progress bars and loading
     *  timeouts.
     *  @param pHandler Progress callback interface. Pass nullptr to
     *    disable progress reporting.
     *  @note Progress handlers can be used to abort the loading
     *    at almost any time.*/
    void SetProgressHandler(ProgressHandler *pHandler);  //�򵼳������ṩһ���Զ���Ľ��ȴ������

    // -------------------------------------------------------------------
    /** Exports the given scene to a chosen file format. Returns the exported
    * data as a binary blob which you can write into a file or something.
	//���������ĳ�����ѡ����ļ���ʽ����������������Ϊ������blob���أ������Խ���д���ļ����������ݡ�

    * When you're done with the data, simply let the #Exporter instance go
    * out of scope to have it released automatically.
    * @param pScene The scene to export. Stays in possession of the caller,
    *   is not changed by the function.
    * @param pFormatId ID string to specify to which format you want to
    *   export to. Use
    * #GetExportFormatCount / #GetExportFormatDescription to learn which
    *   export formats are available.
    * @param pPreprocessing See the documentation for #Export
    * @return the exported data or nullptr in case of error.
    * @note If the Exporter instance did already hold a blob from
    *   a previous call to #ExportToBlob, it will be disposed.
    *   Any IO handlers set via #SetIOHandler are ignored here.
    * @note Use aiCopyScene() to get a modifiable copy of a previously
    *   imported scene. */
    const aiExportDataBlob *ExportToBlob(const aiScene *pScene, const char *pFormatId,
            unsigned int pPreprocessing = 0u, const ExportProperties *pProperties = nullptr);
    const aiExportDataBlob *ExportToBlob(const aiScene *pScene, const std::string &pFormatId,
            unsigned int pPreprocessing = 0u, const ExportProperties *pProperties = nullptr);

    // -------------------------------------------------------------------
    /** Convenience function to export directly to a file. Use
     *  #SetIOSystem to supply a custom IOSystem to gain fine-grained control
     *  about the output data flow of the export process.
	 //ֱ�ӵ�����һ���ļ���

     * @param pBlob A data blob obtained from a previous call to #aiExportScene. Must not be nullptr.//��û���������
     * @param pPath Full target file name. Target must be accessible. //�ļ���ȫ·��
     * @param pPreprocessing Accepts any choice of the ��#aiPostProcessSteps�� enumerated  //���ܡ����ģ�������Ч��ö�ٱ�־��//�����뺯���е��ù�
     *   flags, but in reality only a subset of them makes sense here. Specifying  //������������볡������assimp��Ĭ��ֵʱ�����õ�
     *   'preprocessing' flags is useful if the input scene does not conform to
     *   Assimp's default conventions as specified in the @link data Data Structures Page @endlink.
     *   In short, this means the geometry data should use a right-handed coordinate systems, face  //��������Ӧ��������ϵ���淨��Ϊ��ʱ�롢UV��ͼ���������Ͻ�
     *   winding should be counter-clockwise and the UV coordinate origin is assumed to be in
     *   the upper left. The ��#aiProcess_MakeLeftHanded��, ��#aiProcess_FlipUVs�� and  //��Щ�������ű���������˸��û�������Ĭ��ֵ��
     *   ��#aiProcess_FlipWindingOrder�� flags are used in the import side to allow users
     *   to have those defaults automatically adapted to their conventions. Specifying those flags
     *   for exporting has the opposite effect, respectively. Some other of the
     *   #aiPostProcessSteps enumerated values may be useful as well, but you'll need
     *   to try out what their effect on the exported file is. Many formats impose//��Щ�������㲻����Ҳһ����ִ�У�����˵�е������ʽֻ֧�����ǻ������㲻���ò���Ҳһ����ִ�С�
     *   their own restrictions on the structure of the geometry stored therein,
     *   so some preprocessing may have little or no effect at all, or may be
     *   redundant as exporters would apply them anyhow. A good example
     *   is triangulation - whilst you can enforce it by specifying
     *   the #aiProcess_Triangulate flag, most export formats support only
     *   triangulate data so they would run the step even if it wasn't requested.
     *
     *   If assimp detects that the input scene was directly taken from the importer side of
     *   the library (i.e. not copied using aiCopyScene and potentially modified afterwards),
     *   any post-processing steps already applied to the scene will not be applied again, unless
     *   they show non-idempotent behavior (#aiProcess_MakeLeftHanded, #aiProcess_FlipUVs and
     *   #aiProcess_FlipWindingOrder).
     * @return AI_SUCCESS if everything was fine.
     * @note Use aiCopyScene() to get a modifiable copy of a previously  //��aicopyscene����֮ǰ�����scene
     *   imported scene.*/
    aiReturn Export(const aiScene *pScene, const char *pFormatId, const char *pPath,
            unsigned int pPreprocessing = 0u, const ExportProperties *pProperties = nullptr);
    aiReturn Export(const aiScene *pScene, const std::string &pFormatId, const std::string &pPath,
            unsigned int pPreprocessing = 0u, const ExportProperties *pProperties = nullptr);//���ص��õ����溯��

    // -------------------------------------------------------------------
    /** Returns an error description of an error that occurred in #Export
     *    or #ExportToBlob
     *
     * Returns an empty string if no error occurred.
     * @return A description of the last error, an empty string if no
     *   error occurred. The string is never nullptr.
     *
     * @note The returned function remains valid until one of the
     * following methods is called: #Export, #ExportToBlob, #FreeBlob */
    const char *GetErrorString() const; //���ش�������

    // -------------------------------------------------------------------
    /** Return the blob obtained from the last call to #ExportToBlob */
    const aiExportDataBlob *GetBlob() const;//������һ�λص��� #ExportToBlob��blob

    // -------------------------------------------------------------------
    /** Orphan the blob from the last call to #ExportToBlob. This means
     *  the caller takes ownership and is thus responsible for calling
     *  the C API function #aiReleaseExportBlob to release it. */
    const aiExportDataBlob *GetOrphanedBlob() const;//������blob?

    // -------------------------------------------------------------------
    /** Frees the current blob.
     *
     *  The function does nothing if no blob has previously been
     *  previously produced via #ExportToBlob. #FreeBlob is called
     *  automatically by the destructor. The only reason to call
     *  it manually would be to reclaim as much storage as possible
     *  without giving up the #Exporter instance yet. */
    void FreeBlob();//�ͷŵ�ǰblob

    // -------------------------------------------------------------------
    /** Returns the number of export file formats available in the current
     *  Assimp build. Use #Exporter::GetExportFormatDescription to
     *  retrieve infos of a specific export format.
     *
     *  This includes built-in exporters as well as exporters registered
     *  using #RegisterExporter.
     **/
    size_t GetExportFormatCount() const;//���ص�ǰassimp�ɵ������ļ���ʽ����

    // -------------------------------------------------------------------
    /** Returns a description of the nth export file format. Use #
     *  #Exporter::GetExportFormatCount to learn how many export
     *  formats are supported.
     *
     * The returned pointer is of static storage duration if the
     * pIndex pertains to a built-in exporter (i.e. one not registered
     * via #RegistrerExporter). It is restricted to the life-time of the
     * #Exporter instance otherwise.
     *
     * @param pIndex Index of the export format to retrieve information
     *  for. Valid range is 0 to #Exporter::GetExportFormatCount
     * @return A description of that specific export format.
     *  NULL if pIndex is out of range. */
    const aiExportFormatDesc *GetExportFormatDescription(size_t pIndex) const;//���ص�n�ֵ����ļ���ʽ��������

    // -------------------------------------------------------------------
    /** Register a custom exporter. Custom export formats are limited to
     *    to the current #Exporter instance and do not affect the
     *    library globally. The indexes under which the format's
     *    export format description can be queried are assigned
     *    monotonously.
     *  @param desc Exporter description.
     *  @return aiReturn_SUCCESS if the export format was successfully
     *    registered. A common cause that would prevent an exporter
     *    from being registered is that its format id is already
     *    occupied by another format. */
    aiReturn RegisterExporter(const ExportFormatEntry &desc);//�Զ���һ��������ʽ

    // -------------------------------------------------------------------
    /** Remove an export format previously registered with #RegisterExporter
     *  from the #Exporter instance (this can also be used to drop
     *  built-in exporters because those are implicitly registered
     *  using #RegisterExporter).
     *  @param id Format id to be unregistered, this refers to the
     *    'id' field of #aiExportFormatDesc.
     *  @note Calling this method on a format description not yet registered
     *    has no effect.*/
    void UnregisterExporter(const char *id);  //��exportsʵ����ɾ����ǰ����# registerexporters��ע��ĵ�����ʽ

protected:
    // Just because we don't want you to know how we're hacking around.
    ExporterPimpl *pimpl;//pointer-import-??
};

class ASSIMP_API ExportProperties { //������Զ��ĵȲ���
public:
    // Data type to store the key hash
    typedef unsigned int KeyType;//���ڼ�¼��š�

    // typedefs for our four configuration maps.//����4�����͵�����map
    // We don't need more, so there is no need for a generic solution
	//496�и������塣
    typedef std::map<KeyType, int> IntPropertyMap; //int�������Զ�
    typedef std::map<KeyType, ai_real> FloatPropertyMap;
    typedef std::map<KeyType, std::string> StringPropertyMap;
    typedef std::map<KeyType, aiMatrix4x4> MatrixPropertyMap;
    typedef std::map<KeyType, std::function<void *(void *)>> CallbackPropertyMap;

public:
    /** Standard constructor//���캯��
    * @see ExportProperties()
    */
    ExportProperties();

    // -------------------------------------------------------------------
    /** Copy constructor.//��������
     *
     * This copies the configuration properties of another ExportProperties.
     * @see ExportProperties(const ExportProperties& other)
     */
    ExportProperties(const ExportProperties &other);

    // -------------------------------------------------------------------
    /** Set an integer configuration property. //���� ����int �������ԡ�
     * @param szName Name of the property. All supported properties
     *   are defined in the aiConfig.g header (all constants share the
     *   prefix AI_CONFIG_XXX and are simple strings).
	 //szName ������������֧�ֵ����Զ�������aiConfig.g�����г�������ǰ׺AI_CONFIG_XXX�����Ǽ��ַ���)��
     * @param iValue New value of the property
	 //iValue ������ֵ
     * @return true if the property was set before. The new value replaces
     *   the previous value in this case.
	 //�����˸��ķ���true
     * @note Property of different types (float, int, string ..) are kept
     *   on different stacks, so calling SetPropertyInteger() for a
     *   floating-point property has no effect - the loader will call
     *   GetPropertyFloat() to read the property, but it won't be there.
	 //ע�⣺flout�ȵ����������ͺ�����
     */
    bool SetPropertyInteger(const char *szName, int iValue);

    // -------------------------------------------------------------------
    /** Set a boolean configuration property. Boolean properties
     *  are stored on the integer stack internally so it's possible
     *  to set them via #SetPropertyBool and query them with
     *  #GetPropertyBool and vice versa.
     * @see SetPropertyInteger()
	 //����bool��������
     */
    bool SetPropertyBool(const char *szName, bool value) {
        return SetPropertyInteger(szName, value);
    }

    // -------------------------------------------------------------------
    /** Set a floating-point configuration property.
     * @see SetPropertyInteger()
	 //����float��������
     */
    bool SetPropertyFloat(const char *szName, ai_real fValue);

    // -------------------------------------------------------------------
    /** Set a string configuration property.
     * @see SetPropertyInteger()
	 //����string��������
     */
    bool SetPropertyString(const char *szName, const std::string &sValue);

    // -------------------------------------------------------------------
    /** Set a matrix configuration property.
     * @see SetPropertyInteger()
	 //���� ���� ��������
     */
    bool SetPropertyMatrix(const char *szName, const aiMatrix4x4 &sValue);
    
    bool SetPropertyCallback(const char *szName, const std::function<void *(void *)> &f);

    // -------------------------------------------------------------------
    /** Get a configuration property.  //��ȡ�������Ե�ֵ
     * @param szName Name of the property. All supported properties
     *   are defined in the aiConfig.g header (all constants share the
     *   prefix AI_CONFIG_XXX).
	 //szName 
     * @param iErrorReturn Value that is returned if the property
     *   is not found.
     * @return Current value of the property
     * @note Property of different types (float, int, string ..) are kept
     *   on different lists, so calling SetPropertyInteger() for a
     *   floating-point property has no effect - the loader will call
     *   GetPropertyFloat() to read the property, but it won't be there.
     */
    int GetPropertyInteger(const char *szName,
            int iErrorReturn = 0xffffffff) const;

    // -------------------------------------------------------------------
    /** Get a boolean configuration property. Boolean properties
     *  are stored on the integer stack internally so it's possible
     *  to set them via #SetPropertyBool and query them with
     *  #GetPropertyBool and vice versa.
     * @see GetPropertyInteger()
     */
    bool GetPropertyBool(const char *szName, bool bErrorReturn = false) const {
        return GetPropertyInteger(szName, bErrorReturn) != 0;
    }

    // -------------------------------------------------------------------
    /** Get a floating-point configuration property
     * @see GetPropertyInteger()
     */
    ai_real GetPropertyFloat(const char *szName,
            ai_real fErrorReturn = 10e10f) const;

    // -------------------------------------------------------------------
    /** Get a string configuration property
     *
     *  The return value remains valid until the property is modified.
     * @see GetPropertyInteger()
     */
    const std::string GetPropertyString(const char *szName,
            const std::string &sErrorReturn = "") const;

    // -------------------------------------------------------------------
    /** Get a matrix configuration property
     *
     *  The return value remains valid until the property is modified.
     * @see GetPropertyInteger()
     */
    const aiMatrix4x4 GetPropertyMatrix(const char *szName,
            const aiMatrix4x4 &sErrorReturn = aiMatrix4x4()) const;

    std::function<void *(void *)> GetPropertyCallback(const char* szName) const;

    // -------------------------------------------------------------------
    /** Determine a integer configuration property has been set.
	//ȷ��int ��������������
    * @see HasPropertyInteger()
     */
    bool HasPropertyInteger(const char *szName) const;

    /** Determine a boolean configuration property has been set.
    * @see HasPropertyBool()
     */
    bool HasPropertyBool(const char *szName) const;

    /** Determine a boolean configuration property has been set.
    * @see HasPropertyFloat()
     */
    bool HasPropertyFloat(const char *szName) const;

    /** Determine a String configuration property has been set.
    * @see HasPropertyString()
     */
    bool HasPropertyString(const char *szName) const;

    /** Determine a Matrix configuration property has been set.
    * @see HasPropertyMatrix()
     */
    bool HasPropertyMatrix(const char *szName) const;

    bool HasPropertyCallback(const char *szName) const;

    /** List of integer properties */
    IntPropertyMap mIntProperties;//int���Զ�

    /** List of floating-point properties */
    FloatPropertyMap mFloatProperties;

    /** List of string properties */
    StringPropertyMap mStringProperties;

    /** List of Matrix properties */
    MatrixPropertyMap mMatrixProperties;

    CallbackPropertyMap mCallbackProperties;
};

// ----------------------------------------------------------------------------------
inline const aiExportDataBlob *Exporter::ExportToBlob(const aiScene *pScene, const std::string &pFormatId,
        unsigned int pPreprocessing, const ExportProperties *pProperties) {
    return ExportToBlob(pScene, pFormatId.c_str(), pPreprocessing, pProperties);
}

// ----------------------------------------------------------------------------------
//.cpp��356�к������أ���return�ĺ���
inline aiReturn Exporter ::Export(const aiScene *pScene, const std::string &pFormatId,
        const std::string &pPath, unsigned int pPreprocessing,
        const ExportProperties *pProperties) {
    return Export(pScene, pFormatId.c_str(), pPath.c_str(), pPreprocessing, pProperties);
}

} // namespace Assimp

#endif // ASSIMP_BUILD_NO_EXPORT
#endif // AI_EXPORT_HPP_INC

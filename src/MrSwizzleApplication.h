//------------------------------------------------------------------------------------//
//                                                                                    //
//            _____         _________       .__               .__                     //
//           /     \_______/   _____/_  _  _|__|______________|  |   ____             //
//          /  \ /  \_  __ \_____  \\ \/ \/ /  \___   /\___   /  | _/ __ \            //
//         /    Y    \  | \/        \\     /|  |/    /  /    /|  |_\  ___/            //
//         \____|__  /__| /_______  / \/\_/ |__/_____ \/_____ \____/\___  >           //
//                 \/             \/                 \/      \/         \/            //
//                                                                                    //
//    MrSwizzle is provided under the MIT License(MIT)                                //
//    MrSwizzle uses portions of other open source software.                          //
//    Please review the LICENSE file for further details.                             //
//                                                                                    //
//    Copyright(c) 2015 Matt Davidson                                                 //
//                                                                                    //
//    Permission is hereby granted, free of charge, to any person obtaining a copy    //
//    of this software and associated documentation files(the "Software"), to deal    //
//    in the Software without restriction, including without limitation the rights    //
//    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell     //
//    copies of the Software, and to permit persons to whom the Software is           //
//    furnished to do so, subject to the following conditions :                       //
//                                                                                    //
//    1. Redistributions of source code must retain the above copyright notice,       //
//    this list of conditions and the following disclaimer.                           //
//    2. Redistributions in binary form must reproduce the above copyright notice,    //
//    this list of conditions and the following disclaimer in the                     //
//    documentation and / or other materials provided with the distribution.          //
//    3. Neither the name of the copyright holder nor the names of its                //
//    contributors may be used to endorse or promote products derived                 //
//    from this software without specific prior written permission.                   //
//                                                                                    //
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      //
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        //
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE      //
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          //
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   //
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN       //
//    THE SOFTWARE.                                                                   //
//                                                                                    //
//------------------------------------------------------------------------------------//

#ifndef INCLUDED_APPLICATION
#define INCLUDED_APPLICATION

#include <CtrApplication.h>
#include <CtrPlatform.h>
#include <CtrTimer.h>
#include <CtrLog.h>
#include <CtrNode.h>
#include <CtrTypedProperty.h>
#include <CtrMaterial.h>
#include <CtrImageFunctionNode.h>

namespace Ctr
{
class Window;
class DeviceD3D11;
class Camera;
class ShaderMgr;
class VertexDeclarationMgr;
class InputManager;
class PostEffectsMgr;
class TextureMgr;
class ShaderParameterValueFactory;
class Scene;
class ColorPass;
class IBLRenderPass;
class FocusedDampenedCamera;
class RenderHUD;
class IBLProbe;
class Entity;
class Titles;

class ImageInvertNode;

enum ResolutionType
{
    TwoFiftySix,
    FiveTwelve,
    OneK,
    TwoK
};

// Default options, loaded first from application config,
// and then overwritten by command line options.
class Options
{
  public:
    Options();


    bool        _glossToRoughness;
    bool        _normalizeNormalMapMips;
    bool        _generateMips;
    bool        _generateToksvig;

    float       _textureUVScale;
    int         _processingColorSpace;
    int         _sourceResolution;
    int         _dstResolution;
    std::string _gameTexturesArchive;
    std::string _textureSetAlbedoPathName;
    std::string _environmentPathName;
    bool        _headless;
    float       _metalMaskR;
    float       _metalMaskG;
    float       _metalMaskB;
    float       _metalMaskA;

    std::string _albedoPattern;
    std::string _normalPattern;
    std::string _glossPattern;
    std::string _metalMaskPattern;
    std::string _specularPattern;
    std::string _aoPattern;
    std::string _rmcPattern;
    std::string _roughnessPattern;
    std::string _heightPattern;

    std::string _outputEnvFilePath;
    std::string _outputAlbedoPBRPatternFilePath;
};

// Assemble
class GameTexturesSwizzleGraph : public Node
{
  public:
    GameTexturesSwizzleGraph(Ctr::IDevice* device, 
                             std::shared_ptr<Options> options,
                             const std::string& filename);
    ~GameTexturesSwizzleGraph();

    bool                 setupArchivePaths(const std::string& archivePathFileName);
    bool                 loadTextureSetFromDir(const std::string& filePathName);
    bool                 loadImage(InterpretPixelsAsType pixelType,
                                   const std::string& filePathName);

    HashProperty*        _hashProperty;
    IntProperty*         _gameTexturesSrcResolutionProperty;
    IntProperty*         _commonSrcResolutionProperty;
    Vector2iProperty*    _sourceImageCommonResolutionProperty;
    BoolProperty*        _generateMipMapsProperty;
    IntProperty*         _convertGammaProperty;
    FloatProperty*       _gammaProperty;

    bool                 saveTexture(const ITexture* texture,
                                     const std::string& filenamePrototype,
                                     int32_t mipId);
    void                 saveImageSet(const std::string& textureSet);

    ImageFileSourceNode* _albedoImageNode;
    ImageFileSourceNode* _normalImageNode;
    ImageFileSourceNode* _glossImageNode;
    ImageFileSourceNode* _metalMaskImageNode;
    ImageFileSourceNode* _specularImageNode;
    ImageFileSourceNode* _aoImageNode;
    ImageFileSourceNode* _heightImageNode;


    ComputeAlbedoImageNode *    _computeAlbedoNode;
    ConvertTangentNormalNode *  _convertTangentNormalNode;
    ExtractMetalnessImageNode * _extractMetalnessNode;
    RoughnessImageNode *        _convertRoughnessImageNode;
    ImageMergeNode *            _specularRMCOutputNode;
    ScaleImageNode *            _roughnessRescaleNode;
    ScaleImageNode *            _metalRescaleNode;

    std::shared_ptr<Options>    _options;
};

class MrSwizzleApplication : public Application
{ 
  public:
    MrSwizzleApplication(ApplicationHandle instance);
    virtual ~MrSwizzleApplication();

    bool                       parseOptions(int argc, char* argv[]);
    void                       initialize();
    void                       run();

    ApplicationHandle          instance() const;

    const Timer&               timer() const;
    Timer&                     timer();

    Window*                    window();
    const Window*              window() const;

    bool                       loadParameters();
    bool                       saveParameters() const;

    bool                       loadEnvironment(const std::string& filePathName);
    bool                       saveImages(const std::string& filePathName, bool gameOnly = false);

    void                       loadAsset(Entity*& targetEntity,
                                         const std::string& assetPathName,
                                         const std::string& materialPathName,
                                         bool  userAsset);

    IntProperty*               modelVisualizationProperty();
    FloatProperty*             constantRoughnessProperty();
    FloatProperty*             constantMetalnessProperty();
    IntProperty*               specularWorkflowProperty();
    IntProperty*               debugTermProperty();
    FloatProperty*             specularIntensityProperty();
    FloatProperty*             roughnessScaleProperty();


    enum VisualizationType
    {
        LDR = 1,
        MDR = 2,
        HDR = 4
    };

    enum ModelVisualizationType
    {
        UserModel = 0,
        ShaderBallModel = 1
    };
        

    void                       pause();
    void                       cancel();
    void                       compute();


    Entity*                    visualizedEntity();
    Entity*                    shaderBallEntity();
    Entity*                    sphereEntity();
    Entity*                    iblSphereEntity();

    IntProperty*               visualizationSpaceProperty();

    void                       setupModelVisibility(Ctr::Entity* entity, bool visibility);
    void                       syncVisualization();

    void                       loadGraph(const std::string& zipName);
    void                       setupMaterials();


    const std::unique_ptr<GameTexturesSwizzleGraph>&  gameTexturesGraph() const;
    std::unique_ptr<GameTexturesSwizzleGraph>&        gameTexturesGraph();


    float                      textureUVScale() const;
    std::string                environmentPathName() const;

  protected:
    void                       updateApplication();
    bool                       purgeMessages() const;
    void                       updateVisualizationType();

  private:
    // Properties:
    IntProperty*               _visualizationSpaceProperty;
    IntProperty*               _currentVisualizationSpaceProperty;

    PixelFormatProperty *      _hdrFormatProperty;
    IntProperty*               _probeResolutionProperty;

    IntProperty*               _modelVisualizationProperty;

    // Management layer.
    InputManager*              _inputMgr;
    FocusedDampenedCamera*     _cameraManager;

    RenderHUD*                 _renderHUD;

    // Render passes
    ColorPass*                 _colorPass;
    IBLRenderPass*             _iblRenderPass;
    IBLProbe*                  _probe;

    Ctr::Scene*                 _scene;

    // Application entities for convenience.
    Entity*                    _shaderBallEntity;
    Entity*                    _sphereEntity;
    Entity*                    _iblSphereEntity;

    uint32_t                   _windowWidth;
    uint32_t                   _windowHeight;
    bool                       _windowed;

    FloatProperty*             _constantRoughnessProperty;
    FloatProperty*             _constantMetalnessProperty;

  protected:
    Ctr::Timer                  _timer;
    bool                        _runTitles;
    std::string                 _defaultAsset;

    IntProperty*                _specularWorkflowProperty;
    FloatProperty*              _specularIntensityProperty;
    FloatProperty*              _roughnessScaleProperty;
    IntProperty*                _debugTermProperty;

    std::unique_ptr<GameTexturesSwizzleGraph>  _gameTexturesGraph;
    std::shared_ptr<Options>                   _options;
};
}

#endif
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

#include <MrSwizzleApplication.h>
#include <MrSwizzleApplicationHUD.h>
#include <CtrAssetManager.h>
#include <CtrRenderDeviceD3D11.h>
#include <CtrColorPass.h>
#include <CtrTextureMgr.h>
#include <CtrShaderMgr.h>
#include <CtrRenderHUD.h>
#include <CtrCamera.h>
#include <CtrScene.h>
#include <CtrEntity.h>
#include <CtrMaterial.h>
#include <CtrInputManager.h>
#include <CtrCameraManager.h>
#include <CtrIBLRenderPass.h>
#include <CtrPostEffectsMgr.h>
#include <CtrIBLProbe.h>
#include <CtrTitles.h>
#include <CtrBrdf.h>
#include <CtrImageWidget.h>
#include <Ctrimgui.h>
#include <Ctrimgui.h>
#include <CtrImageFunctionNode.h>

#include <strstream>
#include <CmdLine.h>

namespace Ctr
{

bool
splitFilePathName(const std::string& filePathName,
                  const std::string& fileNameEndPattern,
                  std::string& pathName,
                  std::string& fileName)
{
    size_t forwardSlash = filePathName.rfind("\\");
    size_t backSlash = filePathName.rfind("/");
    size_t fileNameEnd = filePathName.rfind(fileNameEndPattern);
    size_t filenameStart = std::string::npos;

    if (forwardSlash != std::string::npos)
    {
        filenameStart = forwardSlash;
    }
    else if (backSlash != std::string::npos)
    {
        filenameStart = backSlash;
    }
    else
    {
        return false;
    }
    if (fileNameEnd == std::string::npos)
        return false;

    pathName = filePathName.substr(0, filenameStart+1);
    fileName =
        filePathName.substr(filenameStart + 1, fileNameEnd - (filenameStart + 1));

    return true;
}

bool
splitFileExtensionName(const std::string& filePathName,
                       std::string& fileName,
                       std::string& extensionName)
{
    size_t forwardSlash = filePathName.rfind("\\");
    size_t backSlash = filePathName.rfind("/");
    size_t fileNameEnd = filePathName.rfind(".");
    size_t filenameStart = std::string::npos;

    if (forwardSlash != std::string::npos)
    {
        filenameStart = forwardSlash;
    }
    else if (backSlash != std::string::npos)
    {
        filenameStart = backSlash;
    }
    else
    {
        return false;
    }
    if (fileNameEnd == std::string::npos)
        return false;

    fileName = filePathName.substr(filenameStart + 1, fileNameEnd - (filenameStart + 1));
    extensionName = filePathName.substr(fileNameEnd, filePathName.length() - fileNameEnd);
    return true;
}


ImguiEnumVal GameTextureResolutionType[] =
{
    { Ctr::ResolutionType::TwoFiftySix, "256" },
    { Ctr::ResolutionType::FiveTwelve, "512" },
    { Ctr::ResolutionType::OneK, "1024" },
    { Ctr::ResolutionType::TwoK, "2048" }
};
static const EnumTweakType GameTexturesResolutionEnumType(&GameTextureResolutionType[0], 4, "GameTextureType");

enum ProcessingColorSpace
{
    ProcessGammaSpace = 0,
    ProcessLinear = 1
};

ImguiEnumVal GammaEnumTypes[] =
{
    { ProcessGammaSpace, "Gamma" },
    { ProcessLinear, "Linear" },
};
static const EnumTweakType GammaEnumType(&GammaEnumTypes[0], 2, "GammaType");

static uint32_t stringToColorSpace(const std::string& colorSpace)
{
    if (colorSpace == "Gamma")
        return ProcessGammaSpace;
    else
        return ProcessLinear;
}

static std::string colorSpaceToString(uint32_t colorSpace)
{
    if (colorSpace == ProcessGammaSpace)
        return "Gamma";
    else
        return "Linear";
}

static uint32_t stringToResolution(const std::string& resolution)
{
    if (resolution == "256")
        return ResolutionType::TwoFiftySix;
    else if (resolution == "512")
        return ResolutionType::FiveTwelve;
    else if (resolution == "1024")
        return ResolutionType::OneK;
    else
        return ResolutionType::TwoK;
}

static std::string resolutionToString(uint32_t resolution)
{
    if (resolution == ResolutionType::TwoFiftySix)
        return "256";
    else if (resolution == ResolutionType::FiveTwelve)
        return "512";
    else if (resolution == ResolutionType::OneK)
        return "1024";
    else
        return "2048";
}


Options::Options()
{
    _glossToRoughness = false;
    _normalizeNormalMapMips = true;
    _generateMips = true;
    _textureUVScale = 3.0;
    _processingColorSpace = 1;
    _sourceResolution = 3;
    _dstResolution = 3;

    _gameTexturesArchive = "data/textures/Brick_BrownLux_pbr.zip";
    _textureSetAlbedoPathName;
    _outputEnvFilePath;
    _outputAlbedoPBRPatternFilePath;
    _environmentPathName ;

    _headless = false;

    _albedoPattern = "alb";
    _normalPattern = "n";
    _glossPattern = "g";
    _metalMaskPattern = "mask";
    _specularPattern = "s";
    _aoPattern = "ao";
    _rmcPattern = "rmc";
    _roughnessPattern = "r";
    _heightPattern = "h";

    _metalMaskR = 1.0f;
    _metalMaskG = 1.0f;
    _metalMaskB = 0.0f;
    _metalMaskA = 0.0f;
}


GameTexturesSwizzleGraph::GameTexturesSwizzleGraph(Ctr::IDevice* device, 
                                                   std::shared_ptr<Options> options,
                                                   const std::string& filename) :
    _albedoImageNode(nullptr),
    _normalImageNode(nullptr),
    _glossImageNode(nullptr),
    _metalMaskImageNode(nullptr),
    _specularImageNode(nullptr),
    _aoImageNode(nullptr),
    _heightImageNode(nullptr),
    _convertTangentNormalNode(nullptr),
    _computeAlbedoNode(nullptr),
    _extractMetalnessNode(nullptr),
    _specularRMCOutputNode(nullptr),
    _convertRoughnessImageNode(nullptr),
    _roughnessRescaleNode(nullptr),
    _metalRescaleNode(nullptr),
    _hashProperty(new HashProperty(this, "archiveHash")),
    _gameTexturesSrcResolutionProperty(new IntProperty(this, "gtsrcres", new TweakFlags(&GameTexturesResolutionEnumType, "Resolution"))),
    _commonSrcResolutionProperty(new IntProperty(this, "srcres", new TweakFlags(&GameTexturesResolutionEnumType, "Resolution"))),
    _sourceImageCommonResolutionProperty(new Vector2iProperty(this, "commonResolution")),
    _generateMipMapsProperty(new BoolProperty(this, "generateMipMaps")),
    _convertGammaProperty(new IntProperty(this, "convertGamma", new TweakFlags(&GammaEnumType, "Gamma"))),
    _gammaProperty(new FloatProperty(this, "gamma")),
    _options(options)
{
    _convertGammaProperty->set(_options->_processingColorSpace);
    if (_convertGammaProperty->get() == ProcessLinear)
        _gammaProperty->set(2.2f);
    else
        _gammaProperty->set(1.0f);

    _sourceImageCommonResolutionProperty->set(Ctr::Vector2i(2048, 2048));
    _gameTexturesSrcResolutionProperty->set(ResolutionType::TwoK);
    _commonSrcResolutionProperty->set(ResolutionType::TwoK);
    _generateMipMapsProperty->set(false);

    _albedoImageNode = new ImageFileSourceNode(device);
    _normalImageNode = new ImageFileSourceNode(device);
    _glossImageNode = new ImageFileSourceNode(device);
    _metalMaskImageNode = new ImageFileSourceNode(device);
    _specularImageNode = new ImageFileSourceNode(device);
    _aoImageNode = new ImageFileSourceNode(device);
    _computeAlbedoNode = new ComputeAlbedoImageNode(device);
    _convertTangentNormalNode = new ConvertTangentNormalNode(device);
    _convertRoughnessImageNode = new RoughnessImageNode(device);
    _extractMetalnessNode = new ExtractMetalnessImageNode(device);
    _specularRMCOutputNode = new ImageMergeNode(device);
    _metalRescaleNode = new ScaleImageNode(device);
    _roughnessRescaleNode = new ScaleImageNode(device);


    // Setup common size for sources.
    dynamic_cast<Vector2iProperty*>(_albedoImageNode->property("commonSize"))->addDependency(_sourceImageCommonResolutionProperty);
    dynamic_cast<Vector2iProperty*>(_normalImageNode->property("commonSize"))->addDependency(_sourceImageCommonResolutionProperty);
    dynamic_cast<Vector2iProperty*>(_glossImageNode->property("commonSize"))->addDependency(_sourceImageCommonResolutionProperty);
    dynamic_cast<Vector2iProperty*>(_metalMaskImageNode->property("commonSize"))->addDependency(_sourceImageCommonResolutionProperty);
    dynamic_cast<Vector2iProperty*>(_specularImageNode->property("commonSize"))->addDependency(_sourceImageCommonResolutionProperty);
    dynamic_cast<Vector2iProperty*>(_aoImageNode->property("commonSize"))->addDependency(_sourceImageCommonResolutionProperty);

    // Setup mipmap generation
    dynamic_cast<BoolProperty*>(_convertTangentNormalNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_computeAlbedoNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_convertTangentNormalNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_convertRoughnessImageNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_extractMetalnessNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_specularRMCOutputNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_metalRescaleNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_roughnessRescaleNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<BoolProperty*>(_metalMaskImageNode->property("generateMipMaps"))->addDependency(_generateMipMapsProperty);
    dynamic_cast<FloatProperty*>(_albedoImageNode->property("gammaIn"))->addDependency(_gammaProperty);
    dynamic_cast<FloatProperty*>(_specularImageNode->property("gammaIn"))->addDependency(_gammaProperty);
    dynamic_cast<FloatProperty*>(_computeAlbedoNode->property("gammaDisplay"))->addDependency(_gammaProperty);

    _metalMaskImageNode->interpretPixelsAsProperty()->set(MetalMaskImage);
    _albedoImageNode->interpretPixelsAsProperty()->set(AlbedoImage);
    _normalImageNode->interpretPixelsAsProperty()->set(TangentNormalImage);
    _glossImageNode->interpretPixelsAsProperty()->set(GlossImage);
    _metalMaskImageNode->interpretPixelsAsProperty()->set(MetalMaskImage);
    _specularImageNode->interpretPixelsAsProperty()->set(SpecularImage);
    _aoImageNode->interpretPixelsAsProperty()->set(AOImage);
    _computeAlbedoNode->interpretPixelsAsProperty()->set(AlbedoImage);
    _convertTangentNormalNode->interpretPixelsAsProperty()->set(TangentNormalImage);
    _convertRoughnessImageNode->interpretPixelsAsProperty()->set(RoughnessImage);
    _extractMetalnessNode->interpretPixelsAsProperty()->set(MetalImage);
    _specularRMCOutputNode->interpretPixelsAsProperty()->set(UnknownImage);
    _metalRescaleNode->interpretPixelsAsProperty()->set(MetalImage);
    _roughnessRescaleNode->interpretPixelsAsProperty()->set(RoughnessImage);

    if (_options->_textureSetAlbedoPathName.length() > 0)
        loadTextureSetFromDir(_options->_textureSetAlbedoPathName);
    else
        setupArchivePaths(_options->_gameTexturesArchive);

    _gameTexturesSrcResolutionProperty->set(_options->_sourceResolution);
    _commonSrcResolutionProperty->set(_options->_dstResolution);
    int32_t sourceSize = 0;
    switch (_commonSrcResolutionProperty->get())
    {
    case TwoFiftySix:
        sourceSize = 256;
        break;
    case FiveTwelve:
        sourceSize = 512;
        break;
    case OneK:
        sourceSize = 1024;
        break;
    case TwoK:
        sourceSize = 2048;
        break;
    default:
        break;
    }
    _sourceImageCommonResolutionProperty->set(Ctr::Vector2i(sourceSize, sourceSize));

    
    // Todo, XML on all of this.
    _generateMipMapsProperty->set(_options->_generateMips);
    _convertRoughnessImageNode->imageFunctionProperty()->glossToRoughnessProperty()->set(options->_glossToRoughness);
    _convertTangentNormalNode->imageFunctionProperty()->normalizeMipsProperty()->set(_options->_normalizeNormalMapMips);
    _extractMetalnessNode->imageFunctionProperty()->metalnessMaskProperty()->set(Vector4f(_options->_metalMaskR, _options->_metalMaskG, _options->_metalMaskB, _options->_metalMaskA));

    // Setup graph evaluation for PBR generation BEGIN.
    _extractMetalnessNode->setImageDependency(0, _metalMaskImageNode->imageResultProperty());

    _metalRescaleNode->setImageDependency(0, _extractMetalnessNode->imageResultProperty());
    _computeAlbedoNode->setImageDependency(0, _albedoImageNode->imageResultProperty());
    _computeAlbedoNode->setImageDependency(1, _specularImageNode->imageResultProperty());
    _computeAlbedoNode->setImageDependency(2, _metalRescaleNode->imageResultProperty());

    _convertRoughnessImageNode->setImageDependency(0, _glossImageNode->imageResultProperty());
    _roughnessRescaleNode->setImageDependency(0, _convertRoughnessImageNode->imageResultProperty());

    // Default component merge is just R,R,R,R, which will work just fine for this.
    _specularRMCOutputNode->setImageDependency(0, _roughnessRescaleNode->imageResultProperty());
    _specularRMCOutputNode->setImageDependency(1, _metalRescaleNode->imageResultProperty());
    _specularRMCOutputNode->setImageDependency(2, _aoImageNode->imageResultProperty());
    _specularRMCOutputNode->setImageDependency(3, _aoImageNode->imageResultProperty());

    _convertTangentNormalNode->setImageDependency(0, _normalImageNode->imageResultProperty());
    // Setup graph evaluation for PBR generation END.
}

bool
GameTexturesSwizzleGraph::saveTexture(const ITexture* texture,
                                      const std::string& filePathName,
                                      int32_t mipId)
{
    return texture->save(filePathName, true /* fix seams */, false /* split to RGB MMM */, false, mipId);
}

void
GameTexturesSwizzleGraph::saveImageSet(const std::string& filePathNamePrototype)
{
    std::string pathName;
    std::string fileName;
    if (splitFilePathName(filePathNamePrototype,
                          std::string("."),
                          pathName,
                          fileName))
    {
        LOG_CRITICAL("Saving to " << pathName + fileName + "_" + _options->_albedoPattern + ".dds");
        saveTexture(_computeAlbedoNode->textureResultProperty()->get(), pathName + fileName + "_" + _options->_albedoPattern + ".dds", -1);
        saveTexture(_convertTangentNormalNode->textureResultProperty()->get(), pathName + fileName + "_" + _options->_normalPattern + ".dds", -1);
        saveTexture(_specularRMCOutputNode->textureResultProperty()->get(), pathName + fileName + "_" + _options->_rmcPattern + ".dds", -1);

        // TODO: Sep roughness, metalness, height etc... patterns.
        saveTexture(_convertRoughnessImageNode->textureResultProperty()->get(), pathName + fileName + "_" + _options->_roughnessPattern + ".dds", -1);
        saveTexture(_aoImageNode->textureResultProperty()->get(), pathName + fileName + "_" + _options->_aoPattern + ".dds", -1);
        saveTexture(_extractMetalnessNode->textureResultProperty()->get(), pathName + fileName + "_" + _options->_aoPattern + ".dds", -1);
    }
}

bool
GameTexturesSwizzleGraph::loadImage(InterpretPixelsAsType pixelType,    
                                    const std::string& filePathName)
{
    ImageFileSourceNode * fileSourceNode = nullptr;
    switch (pixelType)
    {
        case UnknownImage:
            break;
        case AlbedoImage:
            fileSourceNode = _albedoImageNode;
            break;
        case TangentNormalImage:
            fileSourceNode = _normalImageNode;
            break;
        case SpecularImage:
            fileSourceNode = _specularImageNode;
            break;
        case GlossImage:
            fileSourceNode = _glossImageNode;
            break;
        case MetalMaskImage:
            fileSourceNode = _metalMaskImageNode;
            break;
        case HeightImage:
            fileSourceNode = _heightImageNode;
            break;
        case AOImage:
            fileSourceNode = _aoImageNode;
            break;
        case CavietyImage:
        case ToksvigImage:
        default:
            break;
    };
    if (fileSourceNode != nullptr)
    {
        Ctr::ArchiveHandle handle;
        dynamic_cast<HashProperty*>(fileSourceNode->property("archiveHandle"))->set(handle);
        dynamic_cast<StringProperty*>(fileSourceNode->property("filename"))->set(filePathName);
        return true;
    }
    return false;
}

bool
GameTexturesSwizzleGraph::loadTextureSetFromDir(const std::string& filePathName)
{    
    Ctr::ArchiveHandle handle;
    std::string imageDimension;
    std::string extensionName;
    std::string fileName;
    std::string pathName;

    _options->_textureSetAlbedoPathName = filePathName;

    if (splitFileExtensionName(filePathName, fileName, extensionName) &&
        splitFilePathName(filePathName, _options->_albedoPattern, pathName, fileName))
    {
        LOG_CRITICAL("Loading gametextures using archive base filename: " << fileName);

        LOG("Path = " << filePathName)
        LOG("Filename = " << fileName)

        std::string basePath = fileName;

        // Null handle for all inputs.
        dynamic_cast<HashProperty*>(_albedoImageNode->property("archiveHandle"))->set(handle);
        dynamic_cast<HashProperty*>(_normalImageNode->property("archiveHandle"))->set(handle);
        dynamic_cast<HashProperty*>(_glossImageNode->property("archiveHandle"))->set(handle);
        dynamic_cast<HashProperty*>(_metalMaskImageNode->property("archiveHandle"))->set(handle);
        dynamic_cast<HashProperty*>(_specularImageNode->property("archiveHandle"))->set(handle);
        dynamic_cast<HashProperty*>(_aoImageNode->property("archiveHandle"))->set(handle);

        dynamic_cast<StringProperty*>(_albedoImageNode->property("filename"))->set(pathName + "/" + fileName + _options->_albedoPattern + ".tga");
        dynamic_cast<StringProperty*>(_normalImageNode->property("filename"))->set(pathName + "/" + fileName + _options->_normalPattern + ".tga");
        dynamic_cast<StringProperty*>(_glossImageNode->property("filename"))->set(pathName + "/" + fileName + _options->_glossPattern + ".tga");
        dynamic_cast<StringProperty*>(_metalMaskImageNode->property("filename"))->set(pathName + "/" + fileName + _options->_metalMaskPattern + ".tga");
        dynamic_cast<StringProperty*>(_specularImageNode->property("filename"))->set(pathName + "/" + fileName + _options->_specularPattern + ".tga");
        dynamic_cast<StringProperty*>(_aoImageNode->property("filename"))->set(pathName + "/" + fileName + _options->_aoPattern + ".tga");
        return true;
    }
    return false;
}

bool
GameTexturesSwizzleGraph::setupArchivePaths(const std::string& archivePathFileName)
{
    Ctr::ArchiveHandle handle;
    if (Ctr::AssetManager::assetManager()->openArchive(archivePathFileName, handle))
    {
        _hashProperty->set(handle);
    }
    else
    {
        return false;
    }
    std::string imageDimension;
    _options->_gameTexturesArchive = archivePathFileName;

    switch ((ResolutionType)_gameTexturesSrcResolutionProperty->get())
    {
    case TwoFiftySix:
        imageDimension = "256";
        break;
    case FiveTwelve:
        imageDimension = "512";
        break;
    case OneK:
        imageDimension = "1k";
        break;
    case TwoK:
        imageDimension = "2k";
        break;
    default:
        break;
    }

    std::string pathName;
    std::string fileName;
    splitFilePathName(archivePathFileName, std::string("_"), pathName, fileName);

    LOG("Loading gametextures using archive base filename: " << fileName);

    std::string basePath = "TGA/" + fileName + "_" + imageDimension + "_" + "TGA/" + fileName + "_" +
        imageDimension + std::string("_");

    dynamic_cast<HashProperty*>(_albedoImageNode->property("archiveHandle"))->set(handle);
    dynamic_cast<HashProperty*>(_normalImageNode->property("archiveHandle"))->set(handle);
    dynamic_cast<HashProperty*>(_glossImageNode->property("archiveHandle"))->set(handle);
    dynamic_cast<HashProperty*>(_metalMaskImageNode->property("archiveHandle"))->set(handle);
    dynamic_cast<HashProperty*>(_specularImageNode->property("archiveHandle"))->set(handle);
    dynamic_cast<HashProperty*>(_aoImageNode->property("archiveHandle"))->set(handle);

    dynamic_cast<StringProperty*>(_albedoImageNode->property("filename"))->set(basePath + _options->_albedoPattern + ".tga");
    dynamic_cast<StringProperty*>(_normalImageNode->property("filename"))->set(basePath + _options->_normalPattern + ".tga");
    dynamic_cast<StringProperty*>(_glossImageNode->property("filename"))->set(basePath + _options->_glossPattern + ".tga");
    dynamic_cast<StringProperty*>(_metalMaskImageNode->property("filename"))->set(basePath + _options->_metalMaskPattern + ".tga");
    dynamic_cast<StringProperty*>(_specularImageNode->property("filename"))->set(basePath + _options->_specularPattern + ".tga");
    dynamic_cast<StringProperty*>(_aoImageNode->property("filename"))->set(basePath + _options->_aoPattern + ".tga");


    return true;
}

GameTexturesSwizzleGraph::~GameTexturesSwizzleGraph()
{
    safedelete(_albedoImageNode);
    safedelete(_normalImageNode);
    safedelete(_glossImageNode);
    safedelete(_metalMaskImageNode);
    safedelete(_specularImageNode);
    safedelete(_aoImageNode);
    safedelete(_computeAlbedoNode);
    safedelete(_convertTangentNormalNode);
    safedelete(_convertRoughnessImageNode);
    safedelete(_extractMetalnessNode);
    safedelete(_specularRMCOutputNode);
    safedelete(_roughnessRescaleNode);
    safedelete(_roughnessRescaleNode);
}

namespace
{

ImguiEnumVal IblEnum[] =
{
    { MrSwizzleApplication::HDR, "HDR" },
    { MrSwizzleApplication::MDR, "MDR" },
    { MrSwizzleApplication::LDR, "LDR" }
};

static const EnumTweakType IBLModeType(&IblEnum[0], 3, "IBLMode");


ImguiEnumVal ModelEnum[] =
{
    { MrSwizzleApplication::UserModel, "User" },
    { MrSwizzleApplication::ShaderBallModel, "Shader Ball" },
};
static const EnumTweakType ModelEnumType(&ModelEnum[0], 2, "Model");

// Half arsed for demo.
ImguiEnumVal AppSpecularWorkflowEnum[] =
{
    { RoughnessMetal, "Roughness/Metal" },
    { GlossMetal, "Gloss/Metal" },
    { RoughnessInverseMetal, "Roughness/Inverse Metal" },
    { GlossInverseMetal, "Gloss/Inverse Metal" },
};

static const EnumTweakType WorkflowEnumType(&AppSpecularWorkflowEnum[0], 4, "Workflow");

ImguiEnumVal DebugAOVEnum[] =
{
    { Ctr::NoDebugTerm, "No Debug Term" },
    { Ctr::NormalTerm, "Normals" },
    { Ctr::AmbientOcclusionTerm, "Ambient Occlusion" },
    { Ctr::AlbedoTerm, "Albedo" },
    { Ctr::IBLDiffuseTerm, "IBL Diffuse Radiance" },
    { Ctr::IBLSpecularTerm, "IBL Specular" },
    { Ctr::MetalTerm, "Metalness" },
    { Ctr::RoughnessTerm, "Roughness" },
    { Ctr::BrdfTerm, "Brdf" }
};
static const EnumTweakType DebugAOVType(&DebugAOVEnum[0], 9, "debugAOV");

}

MrSwizzleApplication::MrSwizzleApplication(ApplicationHandle instance) :
    Application(instance),
    _colorPass(nullptr),
    _iblRenderPass(nullptr),
    _cameraManager(nullptr),
    _inputMgr(nullptr),
    _renderHUD(nullptr),
    _sphereEntity(nullptr),
    _iblSphereEntity(nullptr),
    _scene(nullptr),
    _windowWidth (1280),
    _windowHeight(720),
    _windowed (true),
    // TODO: Fix, not sure what th fuck happened here.
    _visualizationSpaceProperty(new IntProperty(this, "Visualization Type")),
    _currentVisualizationSpaceProperty(new IntProperty(this, "Visualization Space")),
    _hdrFormatProperty(new PixelFormatProperty(this, "HDR Format")),
    _probeResolutionProperty(new IntProperty(this, "Visualization Type")),
    _modelVisualizationProperty(new IntProperty(this, "Model", new TweakFlags(&ModelEnumType, "Model"))),
    _constantRoughnessProperty(new FloatProperty(this, "Constant Roughness")),
    _constantMetalnessProperty(new FloatProperty(this, "Constant Metalness")),
    _specularWorkflowProperty(new IntProperty(this, "Specular Workflow", new TweakFlags(&WorkflowEnumType, "Workflow"))),
    _specularIntensityProperty(new FloatProperty(this, "Specular Intensity")),
    _roughnessScaleProperty(new FloatProperty(this, "Roughness Scale")),
    _debugTermProperty(new IntProperty(this, "Debug Visualization", new TweakFlags(&DebugAOVType, "Material"))),
    _defaultAsset("data\\meshes\\pistol\\pistol.obj"),
    _runTitles(false)
{
    _options.reset(new Options());
    _modelVisualizationProperty->set(0);
    _visualizationSpaceProperty->set(Ctr::MrSwizzleApplication::HDR);
    _currentVisualizationSpaceProperty->set(-1);
    _specularIntensityProperty->set(1.0f);
    _roughnessScaleProperty->set(1.0f);

#ifdef _DEBUG
    assert (_instance);
#endif
}

MrSwizzleApplication::~MrSwizzleApplication()
{
    safedelete(_cameraManager);
    safedelete(_colorPass);
    safedelete(_iblRenderPass);
    safedelete(_scene);
    safedelete(_renderHUD);
    safedelete(_inputMgr);
}

std::unique_ptr<GameTexturesSwizzleGraph>&
MrSwizzleApplication::gameTexturesGraph()
{
    return _gameTexturesGraph;
}

const std::unique_ptr<GameTexturesSwizzleGraph>&
MrSwizzleApplication::gameTexturesGraph() const
{
    return _gameTexturesGraph;
}

void
MrSwizzleApplication::loadGraph(const std::string& zipName)
{
    try
    {
        _gameTexturesGraph.reset(new GameTexturesSwizzleGraph(_device, _options, zipName));
        setupMaterials();
    }
    catch (const std::exception& ex)
    {
        // Will most likely assert, but if it keeps going. Log failure and exit.
        LOG_CRITICAL("Failed to load " << zipName << " because " << ex.what())
        exit(0);
    }
}

void
MrSwizzleApplication::setupMaterials()
{
    {
        const std::vector<Mesh*>& meshes = _shaderBallEntity->meshes();
        for (auto meshIt = meshes.begin(); meshIt != meshes.end(); meshIt++)
        {
            // Copy over the last settings.
            (*meshIt)->material()->specularWorkflowProperty()->set(_specularWorkflowProperty->get());
            (*meshIt)->material()->specularIntensityProperty()->set(_specularIntensityProperty->get());
            (*meshIt)->material()->roughnessScaleProperty()->set(_roughnessScaleProperty->get());

            if ((*meshIt)->material()->name().rfind("Brick") != std::string::npos)
            {
                (*meshIt)->material()->albedoMapProperty()->addDependency(_gameTexturesGraph->_computeAlbedoNode->textureResultProperty());
                (*meshIt)->material()->normalMapProperty()->addDependency(_gameTexturesGraph->_convertTangentNormalNode->textureResultProperty());
                (*meshIt)->material()->specularRMCMapProperty()->addDependency(_gameTexturesGraph->_specularRMCOutputNode->textureResultProperty());
                (*meshIt)->material()->textureScaleOffsetProperty()->set(Vector4f(_options->_textureUVScale, _options->_textureUVScale, 0.0f, 0.0f));
            }

        }
    }
}

bool
MrSwizzleApplication::parseOptions(int argc, char* argv[])
{
    // Parse command line options.
    cmdline::parser commandLineOptions;
    {
        // Setup options.
        commandLineOptions.add("help", '\0', "Show Help");
        commandLineOptions.add("headless", '\0', "Run Headless");
    
        // Check for input type.
        commandLineOptions.add<std::string>("gtarchive", '\0', "Specify GameTextures Archive", false, "data/textures/Brick_BrownLux_pbr.zip");
        commandLineOptions.add<std::string>("dirts", '\0', "Specify Texture Set From Albedo PathFileName", false, "");
        commandLineOptions.add<std::string>("envsrc", '\0', "Environment Source Map", false, "");
        commandLineOptions.add<std::string>("envout", '\0', "Environment Output Path and FileName", false, "");
        commandLineOptions.add<std::string>("pbrout", '\0', "PBR output pattern filename", false, "");
    
        if (commandLineOptions.exist("envsrc"))
            _options->_environmentPathName = commandLineOptions.get<std::string>("envsrc");
        if (commandLineOptions.exist("envout"))
            _options->_outputEnvFilePath = commandLineOptions.get<std::string>("envout");
        if (commandLineOptions.exist("pbrout"))
            _options->_outputAlbedoPBRPatternFilePath = commandLineOptions.get<std::string>("pbrout");
    
        // Patterns:
        commandLineOptions.add<std::string>("alb", '\0', "Albedo Filename Pattern", false, "alb");
        commandLineOptions.add<std::string>("norm", '\0', "Tangent Normal Filename Pattern", false, "n");
        commandLineOptions.add<std::string>("gloss", '\0', "Gloss Filename Pattern", false, "g");
        commandLineOptions.add<std::string>("metal", '\0', "Metal Mask Filename Pattern", false, "mask");
        commandLineOptions.add<std::string>("s", '\0', "Specular Color Filename Pattern", false, "specular");
        commandLineOptions.add<std::string>("ao", '\0', "Ambient Occlusion Filename Pattern", false, "ao");
        commandLineOptions.add<std::string>("rmc", '\0', "Roughness - Metalness - Caviety Filename Pattern", false, "rmc");
        commandLineOptions.add<std::string>("height", '\0', "Height Filename Pattern", false, "h");
    
        // Gametextures Input size
        commandLineOptions.add<std::string>("gtres", '\0', "Game Textures Resolution", false, "1024", cmdline::oneof<std::string>("256", "512", "1024", "2048"));
        commandLineOptions.add<std::string>("commonres", '\0', "Common Resolution", false, "1024", cmdline::oneof<std::string>("256", "512", "1024", "2048"));
        commandLineOptions.add<float>("umr", '\0', "Use Metal Mask R", false, 1.0f);
        commandLineOptions.add<float>("umg", '\0', "Use Metal Mask G", false, 1.0f);
        commandLineOptions.add<float>("umb", '\0', "Use Metal Mask B", false, 0.0f);
        commandLineOptions.add<float>("uma", '\0', "Use Metal Mask A", false, 0.0f);
        commandLineOptions.add<bool>("gm", '\0', "Generate Mipmaps", false, true);
        commandLineOptions.add<bool>("nm", '\0', "Normalize Normal Map Mips", false, true);
        commandLineOptions.add<bool>("gentok", '\0', "Generate Toksvig", false, false);
        commandLineOptions.add<std::string>("cps", '\0', "Color Processing Space", false, "Linear", cmdline::oneof<std::string>("Gamma", "Linear"));
        commandLineOptions.add<float>("uvscale", '\0', "Visualization UV Scale", false, 3.0f);
    }
    
    // Common conversion size.
    LOG_CRITICAL("Mr. Swizzle: Image processor for PBR Textures");
    commandLineOptions.parse(argc, argv);
    if (commandLineOptions.exist("help"))
    {
        LOG_CRITICAL(commandLineOptions.usage());
        return false;
    }
    
    if (commandLineOptions.exist("headless"))
        _options->_headless = true;
    
    if (commandLineOptions.exist("umr"))
        _options->_metalMaskR = commandLineOptions.get<float>("umr");
    if (commandLineOptions.exist("umg"))
        _options->_metalMaskG = commandLineOptions.get<float>("umg");
    if (commandLineOptions.exist("umb"))
        _options->_metalMaskB = commandLineOptions.get<float>("umb");
    if (commandLineOptions.exist("uma"))
        _options->_metalMaskA = commandLineOptions.get<float>("uma");
    if (commandLineOptions.exist("alb"))
        _options->_albedoPattern = commandLineOptions.get<std::string>("alb");
    if (commandLineOptions.exist("norm"))
        _options->_normalPattern = commandLineOptions.get<std::string>("norm");
    if (commandLineOptions.exist("gloss"))
        _options->_glossPattern = commandLineOptions.get<std::string>("gloss");
    if (commandLineOptions.exist("metal"))
        _options->_metalMaskPattern = commandLineOptions.get<std::string>("metal");
    if (commandLineOptions.exist("s"))
        _options->_specularPattern = commandLineOptions.get<std::string>("s");
    if (commandLineOptions.exist("ao"))
        _options->_aoPattern = commandLineOptions.get<std::string>("ao");
    if (commandLineOptions.exist("height"))
        _options->_aoPattern = commandLineOptions.get<std::string>("height");
    if (commandLineOptions.exist("gtarchive"))
        _options->_gameTexturesArchive = commandLineOptions.get<std::string>("gtarchive");
    if (commandLineOptions.exist("dirts"))
        _options->_textureSetAlbedoPathName = commandLineOptions.get<std::string>("dirts");
    if (commandLineOptions.exist("envsrc"))
        _options->_environmentPathName = commandLineOptions.get<std::string>("envsrc");
    if (commandLineOptions.exist("envout"))
        _options->_outputEnvFilePath = commandLineOptions.get<std::string>("envout");
    if (commandLineOptions.exist("pbrout"))
        _options->_outputAlbedoPBRPatternFilePath = commandLineOptions.get<std::string>("pbrout");
    if (commandLineOptions.exist("uvscale"))
        _options->_textureUVScale = commandLineOptions.get<float>("uvscale");
    if (commandLineOptions.exist("gm"))
        _options->_generateMips = commandLineOptions.get<bool>("gm") ? 1 : 0;
    if (commandLineOptions.exist("nm"))
        _options->_normalizeNormalMapMips = commandLineOptions.get<bool>("nm") ? 1 : 0;
    if (commandLineOptions.exist("gentok"))
        _options->_generateToksvig = commandLineOptions.get<bool>("gentok") ? 1 : 0;
    if (commandLineOptions.exist("cps"))
        _options->_processingColorSpace = stringToColorSpace(commandLineOptions.get<std::string>("cps"));
    if (commandLineOptions.exist("commonres"))
        _options->_sourceResolution = stringToResolution(commandLineOptions.get<std::string>("commonres"));
    if (commandLineOptions.exist("gtres"))
        _options->_dstResolution = stringToResolution(commandLineOptions.get<std::string>("gtres"));

    return true;
}

ApplicationHandle  
MrSwizzleApplication::instance() const
{
    return _instance;
}

const Timer&
MrSwizzleApplication::timer() const
{
    return _timer;
}

Timer&
MrSwizzleApplication::timer()
{
    return _timer;
}

IntProperty*
MrSwizzleApplication::modelVisualizationProperty()
{
    return _modelVisualizationProperty;
}

FloatProperty*
MrSwizzleApplication::constantRoughnessProperty()
{
    return _constantRoughnessProperty;
}

FloatProperty*
MrSwizzleApplication::constantMetalnessProperty()
{
    return _constantMetalnessProperty;
}

IntProperty*
MrSwizzleApplication::specularWorkflowProperty()
{
    return _specularWorkflowProperty;
}

IntProperty*
MrSwizzleApplication::debugTermProperty()
{
    return _debugTermProperty;
}

FloatProperty*
MrSwizzleApplication::specularIntensityProperty()
{
    return _specularIntensityProperty;
}

FloatProperty*
MrSwizzleApplication::roughnessScaleProperty()
{
    return _roughnessScaleProperty;
}

const Window*
MrSwizzleApplication::window() const
{
    return _mainWindow;
}

Window*
MrSwizzleApplication::window()
{
    return _mainWindow;
}

void
MrSwizzleApplication::initialize()
{
    DeviceD3D11* device = new DeviceD3D11();
    Ctr::ApplicationRenderParameters deviceParams(this, "Mr. Swizzle", Ctr::Vector2i(_windowWidth, _windowHeight), _windowed, _options->_headless);

    if (device->initialize(deviceParams))
    {
        _device = device;
        imguiCreate(_device);

        _mainWindow = _device->renderWindow();

        _scene = new Ctr::Scene(_device);
        _inputMgr = new InputManager (this);
        _cameraManager = new Ctr::FocusedDampenedCamera(_inputMgr->inputState());
        _cameraManager->create(_scene);
        _cameraManager->setTranslation(Ctr::Vector3f(0, -200, 0));
        _cameraManager->setRotation(Ctr::Vector3f(10, -15, -220));

        ImageLerpNode* lerpNode = new ImageLerpNode(_device);

        loadAsset(_shaderBallEntity, "data\\meshes\\shaderBall\\shaderBall.fbx", "", true);
        setupModelVisibility(_shaderBallEntity, true);

        _sphereEntity = _scene->load("data\\meshes\\sphere\\sphere.obj", 
                                     "data\\meshes\\sphere\\sphere.material");
        _sphereEntity->mesh(0)->scaleProperty()->set(Ctr::Vector3f(10,10,10));

        _iblSphereEntity = _scene->load("data\\meshes\\sphere\\sphere.obj", 
                                        "data\\meshes\\sphere\\iblsphere.material");

        _iblSphereEntity->mesh(0)->scaleProperty()->set(Ctr::Vector3f(10,10,10));

        // Temporary
        loadGraph("");
        // Initialize render passes
        _colorPass = new Ctr::ColorPass(_device);
        _iblRenderPass = new Ctr::IBLRenderPass(_device);

        // Add a probe.
        _probe = _scene->addProbe();
        // Default samples
        _probe->sampleCountProperty()->set(128);
        _probe->samplesPerFrameProperty()->set(128);

        _probe->hdrPixelFormatProperty()->set(_hdrFormatProperty->get()),
        _probe->sourceResolutionProperty()->set(_probeResolutionProperty->get());

        Vector4f maxPixelValue = _iblSphereEntity->mesh(0)->material()->albedoMap()->maxValue();
        _probe->maxPixelRProperty()->set(maxPixelValue.x);
        _probe->maxPixelGProperty()->set(maxPixelValue.y);
        _probe->maxPixelBProperty()->set(maxPixelValue.z);


        if (_options->_environmentPathName.length() > 0)
            loadEnvironment(_options->_environmentPathName);

        // Good to go.
        _renderHUD = new Ctr::ApplicationHUD(this, _device, _inputMgr->inputState(), _scene);
        _renderHUD->create();

        {
            _renderHUD->setLogoVisible(true);
            _renderHUD->logo()->setBlendIn(6.0f);
            _renderHUD->showApplicationUI();
        }
        syncVisualization();
    }
    else
    {
        delete device;
        THROW ("Failed to create render device");
    }
}

void
MrSwizzleApplication::syncVisualization()
{
    setupModelVisibility(_shaderBallEntity, true);
}

void
MrSwizzleApplication::setupModelVisibility(Ctr::Entity* entity, bool visibility)
{
    const std::vector<Mesh*>& meshes = entity->meshes();
    for (auto meshIt = meshes.begin(); meshIt != meshes.end(); meshIt++)
    {
        (*meshIt)->setVisible(visibility);
    }
}

void
MrSwizzleApplication::run()
{
    _timer.startTimer();

    // [MattD][Thermal] Turn on to save your gpu some needless processing.
    #define DO_NOT_FRY_GPU 1
    #if DO_NOT_FRY_GPU
        _timer.setLockFrameCounter(60);
    #endif

    do
    {
        _timer.update();
        _inputMgr->update();
        updateApplication();

        if (_options->_headless)
        {
            // save data and exit.
            if (_options->_outputAlbedoPBRPatternFilePath.length() > 0)
                _gameTexturesGraph->saveImageSet(_options->_outputAlbedoPBRPatternFilePath);
            if (_options->_outputEnvFilePath.length() > 0)
                saveImages(_options->_outputEnvFilePath, true);
            return;
        }
    }
    while (!purgeMessages());
}

bool 
MrSwizzleApplication::purgeMessages() const
{
    MSG msg;
    while(PeekMessage (&msg, 0, 0, 0, PM_REMOVE))
    {
        if(msg.message == WM_QUIT)
            return true;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return false;
}

bool
MrSwizzleApplication::loadParameters()
{
    if (std::unique_ptr<pugi::xml_document> doc = 
        std::unique_ptr<pugi::xml_document>(Ctr::AssetManager::assetManager()->openXmlDocument("data/iblBakerConfig.xml")))
    {
        pugi::xpath_node configNode = doc->select_single_node("/Config");
        if (configNode)
        {
            if (const char* xpathValue = configNode.node().attribute("DefaultAsset").value())
            {
                _defaultAsset = xpathValue;
            }

            if (const char* xpathValue = configNode.node().attribute("WindowWidth").value())
            {
                _windowWidth = atoi(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("WindowHeight").value())
            {
                _windowHeight = atoi(xpathValue);
            }

            if (const char* xpathValue = configNode.node().attribute("Windowed").value())
            {
                _windowed = atoi(xpathValue) == 1 ? true : false;
            }

            if (const char* xpathValue = configNode.node().attribute("Titles").value())
            {
                _runTitles = atoi(xpathValue) == 1 ? true : false;
            }
            

            if (const char* xpathValue = configNode.node().attribute("IBLFormat").value())
            {
                _hdrFormatProperty->set(atoi(xpathValue) == 16 ? PF_FLOAT16_RGBA : PF_FLOAT32_RGBA);
            }

            if (const char* xpathValue = configNode.node().attribute("SourceEnvironmentResolution").value())
            {
                _probeResolutionProperty->set(atoi(xpathValue));
            }

            if (const char* xpathValue = configNode.node().attribute("SpecularWorkflow").value())
            {
                std::string workflowValue(xpathValue);

                SpecularWorkflow workflow;
                if (workflowValue == std::string("RoughnessMetal"))
                {
                    workflow = Ctr::RoughnessMetal;
                }
                else if (workflowValue == std::string("GlossMetal"))
                {
                    workflow = Ctr::GlossMetal;
                }
                else if (workflowValue == std::string("RoughnessInverseMetal"))
                {
                    workflow = Ctr::RoughnessInverseMetal;
                }
                else if (workflowValue == std::string("GlossInverseMetal"))
                {
                    workflow = Ctr::GlossInverseMetal;
                }
                else
                {
                    LOG_WARNING("Could not find a valid preference for specular workflow, using RoughnessMetal.")
                }

                _specularWorkflowProperty->set(workflow);
            }
            else
            {
                LOG_WARNING("Could not locate a preference for specular workflow, using RoughnessMetal.")
            }

            if (const char* xpathValue = configNode.node().attribute("GlossToRoughness").value())
            {
                _options->_glossToRoughness = atoi(xpathValue) == 1 ? true : false;
            }
            if (const char* xpathValue = configNode.node().attribute("NormalizeMipMaps").value())
            {
                _options->_normalizeNormalMapMips = atoi(xpathValue) == 1 ? true : false;
            }
            if (const char* xpathValue = configNode.node().attribute("GenerateMipMaps").value())
            {
                _options->_generateMips = atoi(xpathValue) == 1 ? true : false;
            }
            if (const char* xpathValue = configNode.node().attribute("TextureUVScale").value())
            {
                _options->_textureUVScale = (float)atof(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("EnvironmentPathName").value())
            {
                if (strlen(xpathValue) > 0)
                    _options->_environmentPathName = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("GameTextureArchive").value())
            {
                if (strlen(xpathValue) > 0)
                    _options->_gameTexturesArchive = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("TextureSetAlbedoPathName").value())
            {
                if (strlen(xpathValue) > 0)
                    _options->_textureSetAlbedoPathName = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("OutputEnvFilePath").value())
            {
                if (strlen(xpathValue) > 0)
                    _options->_outputEnvFilePath = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("OutputAlbedoPBRPatternFilePath").value())
            {
                if (strlen(xpathValue) > 0)
                    _options->_outputAlbedoPBRPatternFilePath = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("ProcessingColorSpace").value())
            {
                _options->_processingColorSpace = stringToColorSpace(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("SourceResolution").value())
            {
                _options->_sourceResolution = stringToResolution(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("DstResolution").value())
            {
                _options->_dstResolution = stringToResolution(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("AlbedoPattern").value())
            {
                _options->_albedoPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("NormalPattern").value())
            {
                _options->_normalPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("GlossPattern").value())
            {
                _options->_glossPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("MetalMaskPattern").value())
            {
                _options->_metalMaskPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("SpecularPattern").value())
            {
                _options->_specularPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("AOPattern").value())
            {
                _options->_aoPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("RMCPattern").value())
            {
                _options->_rmcPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("RoughnessPattern").value())
            {
                _options->_roughnessPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("HeightPattern").value())
            {
                _options->_heightPattern = xpathValue;
            }
            if (const char* xpathValue = configNode.node().attribute("MetalMaskR").value())
            {
                _options->_metalMaskR = (float)atof(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("MetalMaskG").value())
            {
                _options->_metalMaskG = (float)atof(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("MetalMaskB").value())
            {
                _options->_metalMaskB = (float)atof(xpathValue);
            }
            if (const char* xpathValue = configNode.node().attribute("MetalMaskA").value())
            {
                _options->_metalMaskA = (float)atof(xpathValue);
            }

            if (_windowWidth <= 0 || _windowHeight <= 0)
            {
                _windowWidth = 1280;
                _windowHeight = 720;
            }

            return true;
        }
    }
    return false;
}

float
MrSwizzleApplication::textureUVScale() const
{
    const std::vector<Mesh*>& meshes = _shaderBallEntity->meshes();
    for (auto meshIt = meshes.begin(); meshIt != meshes.end(); meshIt++)
    {
        if ((*meshIt)->material()->name().rfind("Brick") != std::string::npos)
        {
            return (*meshIt)->material()->textureScaleOffset().x;
        }
    }
    return 1.0f;
}

std::string
MrSwizzleApplication::environmentPathName() const
{
    const std::vector<std::string>& pathNames  =
        _sphereEntity->mesh(0)->material()->albedoMapProperty()->get()->resource()->filenames();
    if (pathNames.size() > 0)
        return pathNames[0];
    return std::string("");
}

bool
MrSwizzleApplication::saveParameters() const
{
    std::unique_ptr<pugi::xml_document> doc;
    doc.reset(new pugi::xml_document());
    if (doc)
    {
        // Quick sanitize for close while minimized.
        int32_t width = window()->width();
        int32_t height = window()->height();

        if (width <= 0 || height <= 0)
        {
            width = 1280;
            height = 720;
        }

        pugi::xml_node configNode = doc->append_child(pugi::node_element);
        configNode.set_name("Config");

        configNode.append_attribute("DefaultAsset").set_value(_defaultAsset.c_str());

        char buffer[512];
        memset(buffer, 0, sizeof(char) * 512);
        _itoa(window()->width(), buffer, 10);
        configNode.append_attribute("WindowWidth").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        _itoa(window()->height(), buffer, 10);
        configNode.append_attribute("WindowHeight").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        _itoa(window()->windowed() ? 1 : 0, buffer, 10);
        configNode.append_attribute("Windowed").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        _itoa(_runTitles ? 1 : 0, buffer, 10);
        configNode.append_attribute("Titles").set_value(buffer);

        int format = _scene->probes()[0]->hdrPixelFormat() == PF_FLOAT16_RGBA ? 16 : 32;
        memset(buffer, 0, sizeof(char) * 512);
        _itoa(format, buffer, 10);
        configNode.append_attribute("IBLFormat").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        _itoa(_scene->probes()[0]->sourceResolutionProperty()->get(), buffer, 10);
        configNode.append_attribute("SourceEnvironmentResolution").set_value(buffer);


        std::string workflow("RoughnessMetal");
        switch (_specularWorkflowProperty->get())
        {
            case Ctr::RoughnessMetal:
                workflow = "RoughnessMetal";
                break;
            case Ctr::GlossMetal:
                workflow = "GlossMetal";
                break;
            case Ctr::RoughnessInverseMetal:
                workflow = "RoughnessInverseMetal";
                break;
            case Ctr::GlossInverseMetal:
                workflow = "GlossInverseMetal";
                break;
        }
        configNode.append_attribute("SpecularWorkflow").set_value(workflow.c_str());

        memset(buffer, 0, sizeof(char) * 512);
        _itoa(_gameTexturesGraph->_convertRoughnessImageNode->imageFunctionProperty()->glossToRoughnessProperty()->get(), buffer, 10);
        configNode.append_attribute("GlossToRoughness").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        _itoa(_gameTexturesGraph->_generateMipMapsProperty->get(), buffer, 10);
        configNode.append_attribute("GenerateMipMaps").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        _itoa(_gameTexturesGraph->_convertTangentNormalNode->imageFunctionProperty()->normalizeMipsProperty()->get(), buffer, 10);
        configNode.append_attribute("NormalizeMipMaps").set_value(buffer);


        memset(buffer, 0, sizeof(char) * 512);
        configNode.append_attribute("ProcessingColorSpace").set_value(colorSpaceToString(_gameTexturesGraph->_convertGammaProperty->get()).c_str());

        memset(buffer, 0, sizeof(char) * 512);
        configNode.append_attribute("SourceResolution").set_value(resolutionToString(_gameTexturesGraph->_gameTexturesSrcResolutionProperty->get()).c_str());
        memset(buffer, 0, sizeof(char) * 512);
        configNode.append_attribute("DstResolution").set_value(resolutionToString(_gameTexturesGraph->_commonSrcResolutionProperty->get()).c_str());
        
        const Vector4f& metalnessMask =
            _gameTexturesGraph->_extractMetalnessNode->imageFunctionProperty()->metalnessMaskProperty()->get();

        memset(buffer, 0, sizeof(char) * 512);
        sprintf(buffer, "%f", metalnessMask.x);
        configNode.append_attribute("MetalMaskR").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        sprintf(buffer, "%f", metalnessMask.y);
        configNode.append_attribute("MetalMaskG").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        sprintf(buffer, "%f", metalnessMask.z);
        configNode.append_attribute("MetalMaskB").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        sprintf(buffer, "%f", metalnessMask.w);
        configNode.append_attribute("MetalMaskA").set_value(buffer);

        memset(buffer, 0, sizeof(char) * 512);
        sprintf(buffer, "%f", textureUVScale());
        configNode.append_attribute("TextureUVScale").set_value(buffer);


        configNode.append_attribute("EnvironmentPathName").set_value(environmentPathName().c_str());
        configNode.append_attribute("AlbedoPattern").set_value(_options->_albedoPattern.c_str());
        configNode.append_attribute("Normal").set_value(_options->_normalPattern.c_str());
        configNode.append_attribute("Gloss").set_value(_options->_glossPattern.c_str());
        configNode.append_attribute("MetalMaskPattern").set_value(_options->_metalMaskPattern.c_str());
        configNode.append_attribute("SpecularPattern").set_value(_options->_specularPattern.c_str());
        configNode.append_attribute("AOPattern").set_value(_options->_aoPattern.c_str());
        configNode.append_attribute("RMCPattern").set_value(_options->_rmcPattern.c_str());
        configNode.append_attribute("RoughnessPattern").set_value(_options->_roughnessPattern.c_str());
        configNode.append_attribute("HeightPattern").set_value(_options->_heightPattern.c_str());
        configNode.append_attribute("GameTextureArchive").set_value(_options->_gameTexturesArchive.c_str());
        configNode.append_attribute("TextureSetAlbedoPathName").set_value(_options->_textureSetAlbedoPathName.c_str());
        configNode.append_attribute("OutputEnvFilePath").set_value(_options->_outputEnvFilePath.c_str());
        configNode.append_attribute("OutputAlbedoPBRPatternFilePath").set_value(_options->_outputAlbedoPBRPatternFilePath.c_str());


        doc->save_file("data/iblBakerConfig.xml");
    }

    return true;
}

void
MrSwizzleApplication::updateApplication()
{
    float elapsedTime = (float)(_timer.elapsedTime());
    _cameraManager->update(elapsedTime, true, true);
    _device->update();
    _scene->update();
    _renderHUD->update(elapsedTime);

    Ctr::InputState* inputState = _inputMgr->input().inputState();
    if (inputState->leftMouseDown() && !inputState->hasGUIFocus() && 
        inputState->getKeyState(DIK_LCONTROL))
    {
        Vector3f rotation;
        Ctr::Entity* entity = shaderBallEntity();

        rotation = entity->mesh(0)->rotation();

        rotation += Ctr::Vector3f(((float)inputState->_y),((float)inputState->_x), 0);
        std::vector<Mesh*> meshes = entity->meshes();
        for (auto it = meshes.begin(); it != meshes.end(); it++)
            (*it)->rotationProperty()->set(rotation);
    }

    if (_visualizationSpaceProperty->get() != _currentVisualizationSpaceProperty->get())
    {
        updateVisualizationType();
    }

    Ctr::Camera* camera = _scene->camera();
    camera->updateViewProjection();
    camera->cacheCameraTransforms();
    _device->beginRender();
    // Perform application work.
    const Ctr::Vector4f clearColor = Ctr::Vector4f(0, 1,0,1);
    _device->bindFrameBuffer (_device->postEffectsMgr()->sceneFrameBuffer());
    _device->clearSurfaces (0, Ctr::CLEAR_TARGET | Ctr::CLEAR_ZBUFFER|Ctr::CLEAR_STENCIL, 
                                clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    if (!_inputMgr->inputState()->leftMouseDown())
        _iblRenderPass->render(_scene);


     // Camera input.
    _device->bindFrameBuffer(_device->postEffectsMgr()->sceneFrameBuffer());
    
    _colorPass->render(_scene);

	 // Finalize post effects.
    _device->postEffectsMgr()->render(camera);

    _device->bindFrameBuffer(_device->deviceFrameBuffer());
    _renderHUD->render(camera);

	// Present to back buffre
    _device->present();
}

bool
MrSwizzleApplication::loadEnvironment(const std::string& filePathName)
{
    bool result = false; 
    if (AssetManager::fileExists(filePathName))
    {
        // TODO: Reference counting.
        _device->textureMgr()->recycle(_sphereEntity->mesh(0)->material()->albedoMap());
        _sphereEntity->mesh(0)->material()->setAlbedoMap(filePathName);
        _iblSphereEntity->mesh(0)->material()->setAlbedoMap(filePathName);

        _scene->probes()[0]->uncache();

        // Is the environment a cubemap, if not, load up spherical versions of shaders.
        if (const ITexture* texture = _sphereEntity->mesh(0)->material()->albedoMap())
        {
            //texture->textureCount
            if (texture->isCubeMap())
            {
                // Setup cubemap shaders.
                _sphereEntity->mesh(0)->material()->setShaderName("EnvironmentSphere");
                _iblSphereEntity->mesh(0)->material()->setShaderName("SinglePassEnvironment");
            }
            else
            {
                // Setup spherical map shaders.
                _sphereEntity->mesh(0)->material()->setShaderName("EnvironmentSphereSpherical");
                _iblSphereEntity->mesh(0)->material()->setShaderName("SinglePassSphericalEnvironment");
            }


            Ctr::PixelFormat format = texture->format();
            float inputGamma = 1.0;
            // Setup default gamma. 1.0 for HDR, 2.2 for LDR
            if (format == Ctr::PF_FLOAT32_RGBA ||
                format == Ctr::PF_FLOAT16_RGBA ||
                format == Ctr::PF_FLOAT32_RGB ||
                format == Ctr::PF_FLOAT16_GR ||
                format == Ctr::PF_FLOAT32_GR ||
                format == Ctr::PF_FLOAT32_R ||
                format == Ctr::PF_FLOAT16_R)
            {
                 inputGamma = 1.0f;
            }
            else
            {
                 inputGamma = 2.2f;
            }

            _iblSphereEntity->mesh(0)->material()->textureGammaProperty()->set(1.0f);
            _sphereEntity->mesh(0)->material()->textureGammaProperty()->set(inputGamma);
        }

        _device->shaderMgr()->resolveShaders(_sphereEntity);
        _device->shaderMgr()->resolveShaders(_iblSphereEntity);


        Vector4f maxPixelValue = _iblSphereEntity->mesh(0)->material()->albedoMap()->maxValue();
        _probe->maxPixelRProperty()->set(maxPixelValue.x);
        _probe->maxPixelGProperty()->set(maxPixelValue.y);
        _probe->maxPixelBProperty()->set(maxPixelValue.z);

    }
    else
    {
        LOG ("Could not open file " << filePathName);
    }

    return result;
}

bool
MrSwizzleApplication::saveImages(const std::string& filePathName, bool gameOnly)
{
    if (Ctr::IBLProbe* probe = _scene->probes()[0])
    {
        bool trimmed = true;

        std::vector <std::string> pmtConversionQueue;

        size_t pathEnd = filePathName.rfind("/");
        if (pathEnd == std::string::npos)
        {
            pathEnd = filePathName.rfind("\\");
            if (pathEnd == std::string::npos)
            {
                LOG_CRITICAL("Failed to find path end in " << filePathName);
                return false;
            }
        }
        size_t extension = filePathName.rfind(".");
        if (extension == std::string::npos)
        {
            extension = filePathName.size();
        }

        std::string pathName = filePathName.substr(0, pathEnd+1);
        std::string fileNameBase = filePathName.substr(pathEnd+1, extension-(pathEnd+1));

        LOG ("PathName " << pathName);
        LOG ("FileName base " << fileNameBase);

        if (trimmed)
        {
            std::string specularHDRPath = pathName + fileNameBase + "SpecularHDR.dds";
            std::string diffuseHDRPath = pathName + fileNameBase + "DiffuseHDR.dds";
            std::string envHDRPath = pathName + fileNameBase + "EnvHDR.dds";

            std::string specularMDRPath = pathName + fileNameBase + "SpecularMDR.dds";
            std::string diffuseMDRPath = pathName + fileNameBase + "DiffuseMDR.dds";
            std::string envMDRPath = pathName + fileNameBase + "EnvMDR.dds";

            std::string brdfLUTPath = pathName + fileNameBase + "Brdf.dds";

            LOG_CRITICAL("Saving RGBM MDR diffuse to " << diffuseMDRPath);
            probe->diffuseCubeMapMDR()->save(diffuseMDRPath, true /* fix seams */, false /* split to RGB MMM */);
            LOG_CRITICAL("Saving RGBM MDR specular to " << specularMDRPath);
            probe->specularCubeMapMDR()->save(specularMDRPath, true /* fix seams */, false /* split to RGB MMM */);
            LOG_CRITICAL("Saving RGBM MDR environment to " << envMDRPath);
            probe->environmentCubeMapMDR()->save(envMDRPath, true /* fix seams */, false /* split to RGB MMM */);


            // Save the brdf too.
            _scene->activeBrdf()->brdfLut()->save(brdfLUTPath, false, false);


// This operation on a 2k floating point cubemap with a full mip chain blows
// through remaining addressable memory on 32bit.
#if _64BIT
            probe->environmentCubeMap()->save(envHDRPath, true, false);
#endif
            LOG_CRITICAL("Saving HDR diffuse to " << diffuseHDRPath);
            probe->diffuseCubeMap()->save(diffuseHDRPath, true, false);

            LOG_CRITICAL("Saving HDR specular to " << specularHDRPath);
            probe->specularCubeMap()->save(specularHDRPath, true, false);

            return true;
        }
    }

    return false;
}

void
MrSwizzleApplication::compute()
{
    if (Ctr::IBLProbe* probe = _scene->probes()[0])
    {
        if (probe->computed())
        {
            probe->uncache();
        }
        else
        {
            probe->markComputed(false);
        }
    }
}

void
MrSwizzleApplication::pause()
{
    throw std::runtime_error("Not implemented!");
}

void
MrSwizzleApplication::cancel()
{
    if (Ctr::IBLProbe* probe = _scene->probes()[0])
    {
        probe->markComputed(true);
    }
}

IntProperty*
MrSwizzleApplication::visualizationSpaceProperty()
{
    return _visualizationSpaceProperty;
}

void
MrSwizzleApplication::updateVisualizationType()
{
    _currentVisualizationSpaceProperty->set(_visualizationSpaceProperty->get());

    switch (_currentVisualizationSpaceProperty->get())
    {
        case MrSwizzleApplication::HDR:
            _shaderBallEntity->mesh(0)->material()->setTechniqueName("Default");
            break;
    }
    _device->shaderMgr()->resolveShaders(_shaderBallEntity);

}

Entity*
MrSwizzleApplication::shaderBallEntity()
{
    return _shaderBallEntity;
}

Entity*
MrSwizzleApplication::sphereEntity()
{
    return _sphereEntity;
}

Entity*
MrSwizzleApplication::iblSphereEntity()
{
    return _iblSphereEntity;
}

void
MrSwizzleApplication::loadAsset(Entity*& targetEntity,
                       const std::string& assetPathName, 
                       const std::string& materialPathName,
                       bool userAsset)
{
    if (targetEntity && userAsset)
    {
        _scene->destroy(targetEntity);
    }

    targetEntity = _scene->load(assetPathName, materialPathName);
    //if (userAsset)
    { 
        const std::vector<Mesh*>& meshes = targetEntity->meshes();
        for (auto meshIt = meshes.begin(); meshIt != meshes.end(); meshIt++)
        {
            // Copy over the last settings.
            (*meshIt)->material()->specularWorkflowProperty()->set(_specularWorkflowProperty->get());
            (*meshIt)->material()->specularIntensityProperty()->set(_specularIntensityProperty->get());
            (*meshIt)->material()->roughnessScaleProperty()->set(_roughnessScaleProperty->get());
        }
    }
}
}
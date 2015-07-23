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

#include <MrSwizzleApplicationHUD.h>
#include <CtrApplication.h>
#include <CtrCamera.h>
#include <CtrScene.h>
#include <CtrEntity.h>
#include <CtrMaterial.h>
#include <CtrMesh.h>
#include <CtrPostEffect.h>
#include <CtrPostEffectsMgr.h>
#include <CtrEntity.h>
#include <CtrMaterial.h>
#include <CtrIblProbe.h>
#include <CtrBrdf.h>
#include <CtrTextureMgr.h>
#include <Ctrimgui.h>
#include <CommDlg.h>
#include <MrSwizzleApplication.h>

namespace Ctr
{
#define MAX_FILE_PATH_NAME 1024

ApplicationHUD* ApplicationHUD::_mrSwizzleApplicationHud = 0;

BOOL selectFilenameLoad(LPWSTR filename, 
                        LPWSTR filter, 
                        LPWSTR title = L"Open File")
{
    OPENFILENAME ofn;       // common dialog box structure
    WCHAR dirName[MAX_FILE_PATH_NAME];

    BOOL fileOpenStatus;

    // Initialize OPENFILENAME structure
    memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_FILE_PATH_NAME;
    ofn.lpstrFilter = filter; 
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.lpstrFileTitle = title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    _wgetcwd(dirName, MAX_FILE_PATH_NAME);
    fileOpenStatus = ::GetOpenFileName(&ofn);
    _wchdir(dirName);

    return fileOpenStatus;
}

BOOL selectFilenameSave(WindowHandle windowHandle, 
    LPWSTR filename, 
    LPWSTR filter,
    LPWSTR title = L"Save As")
{
    OPENFILENAME ofn = { sizeof(OPENFILENAME), windowHandle, NULL,
        filter, NULL, 0, 1,
        filename, MAX_FILE_PATH_NAME, NULL, 0,
        NULL, title, NULL,
        0, 0, NULL,
        0, NULL, NULL };
    BOOL fileSaveStatus;
    WCHAR dirName[MAX_FILE_PATH_NAME];

    _wgetcwd(dirName, MAX_FILE_PATH_NAME);
    fileSaveStatus = ::GetSaveFileName(&ofn);
    _wchdir(dirName);

    return fileSaveStatus;
}

ApplicationHUD::ApplicationHUD (Ctr::MrSwizzleApplication* application,
                                Ctr::IDevice* device,
                                Ctr::InputState* inputState,
                                Ctr::Scene* scene) : 
RenderHUD (application, device, inputState),
_scene (scene),
_controlsVisible (false),
_gameTexture(nullptr),
_showRendering(false),
_renderingEnabled(true),
_showAbout(false),
_aboutEnabled(true),
_showEnvironment(true),
_environmentEnabled(true),
_showBrdf(true),
_brdfEnabled(true),
_showFiltering(true),
_filteringEnabled(true),
_mrSwizzleApplication(application),
_scrollArea(0)
{
    _mrSwizzleApplicationHud = this;
    Ctr::Camera* camera = _scene->camera();

    _controlsVisible = true;
    setUIVisible(true);

    _gameTexture = device->textureMgr()->loadTexture("data/textures/BbTitles/gametextures_logo.dds");
}

void
ApplicationHUD::setupMeshUI()
{
}

void
ApplicationHUD::cleanupMeshUI()
{
}

void
ApplicationHUD::showApplicationUI()
{

}

ApplicationHUD::~ApplicationHUD()
{
}

bool
ApplicationHUD::create()
{
    return RenderHUD::create();
}

bool
ApplicationHUD::update(double elapsedTime)
{
    if (_inputState->getKeyState(DIK_F1))
    {
        _controlsVisible = true;
        setUIVisible (true);
    }
    if (_inputState->getKeyState(DIK_F2))
    {
        _controlsVisible = false;
        setUIVisible (false);
    }

    if (RenderHUD::update(elapsedTime))
    {
        return true;
    }
    return false;
}

uint32_t idFromVals(int32_t value, const ImguiEnumVal* values, uint32_t valueCount)
{
    for (uint32_t valueId = 0; valueId < valueCount; valueId++)
    {
        if (values[valueId].value == value)
        {
            return valueId;
        }
    }

    LOG("Program error: No such value " << value << " in enum representation.");
    return 0;
}

template <class T>
void
chooseForProperty(T* property)
{
    const ImguiEnumVal* enumVals = property->tweakFlags()->enumType->enumValues();
    uint32_t currentValue = property->get();
    uint32_t enumCount = property->tweakFlags()->enumType->enumCount();
    uint32_t currentId = idFromVals(currentValue, enumVals, enumCount);

    uint32_t newId = imguiChooseFromArrayInstead(currentId, enumVals, enumCount);
    if (newId != currentId)
    {
        property->set(enumVals[newId].value);
    }
}

template <class T>
void
chooseForProperties(T** properties, uint32_t numProperties)
{
    const ImguiEnumVal* enumVals = properties[0]->tweakFlags()->enumType->enumValues();
    uint32_t currentValue = properties[0]->get();
    uint32_t enumCount = properties[0]->tweakFlags()->enumType->enumCount();
    uint32_t currentId = idFromVals(currentValue, enumVals, enumCount);

    uint32_t newId = imguiChooseFromArrayInstead(currentId, enumVals, enumCount);
    if (newId != currentId)
    {
        for (uint32_t propertyId = 0; propertyId < numProperties; propertyId++)
            properties[propertyId]->set(enumVals[newId].value);
    }
}

static void imguiRegion(const char* title, const char* strRight, bool& flag, bool enabled = true)
{
    if (imguiCollapse(title, strRight, flag, enabled))
    {
        flag = !flag;
    }
}

void imguiRegionBorder(const char* title, const char* strRight, bool& flag, bool enabled = true)
{
    imguiSeparatorLine(1);
    imguiRegion(title, strRight, flag, enabled);
    imguiSeparatorLine(1);
    imguiSeparator(4);
}

template <class T, class S>
void
imguiPropertySlider(const char* title, T* property, float min, float max, S step, bool enabled = true)
{
    float displayGamma = float(property->get());
    if (imguiSlider(title, displayGamma, min, max, float(step), enabled))
    {
        property->set(S(displayGamma));
    }
}

template <class T, typename S>
void
imguiPropertiesSlider(const char* title, T** properties, uint32_t propertyCount, float min, float max, S step, bool enabled = true)
{
    float value = float(properties[0]->get());
    if (imguiSlider(title, value, min, max, float(step), enabled))
    {
        for (uint32_t propertyId = 0; propertyId < propertyCount; propertyId++)
            properties[propertyId]->set(S(value));
    }
}

template <class T>
void
imguiSelectionSliderForEnumProperty(const char* title, T* property)
{
    const ImguiEnumVal* enumVals = property->tweakFlags()->enumType->enumValues();
    uint32_t currentValue = uint32_t(property->get());
    uint32_t enumCount = property->tweakFlags()->enumType->enumCount();
    uint32_t currentId = uint32_t(idFromVals(currentValue, enumVals, uint32_t(enumCount)));

    float minValue = float(0);
    float maxValue = float(enumCount-1);

    std::vector <const char*> labels;
    labels.reserve(enumCount);
    for (uint32_t labelId = 0; labelId < enumCount; labelId++)
        labels.push_back(enumVals[labelId].label);
    imguiLabel(title);

    const uint8_t tab = imguiTabsForEnum(uint8_t(currentId), true, ImguiAlign::CenterIndented, 16, 2, enumCount, enumVals);
    if (uint32_t(tab) != currentId)
    {
        property->set(enumVals[tab].value);
    }
}

template <class T>
void
imguiSelectionSliderForPixelFormatProperty(const char* title, T* property)
{
    const ImguiEnumVal* enumVals = property->tweakFlags()->enumType->enumValues();
    uint32_t currentValue = uint32_t(property->get());
    uint32_t enumCount = property->tweakFlags()->enumType->enumCount();
    uint32_t currentId = uint32_t(idFromVals(currentValue, enumVals, uint32_t(enumCount)));

    float minValue = float(0);
    float maxValue = float(enumCount - 1);

    std::vector <const char*> labels;
    labels.reserve(enumCount);
    for (uint32_t labelId = 0; labelId < enumCount; labelId++)
        labels.push_back(enumVals[labelId].label);
    imguiLabel(title);

    const uint8_t tab = imguiTabsForEnum(uint8_t(currentId), true, ImguiAlign::CenterIndented, 16, 2, enumCount, enumVals);
    if (uint32_t(tab) != currentId)
    {
        property->set((PixelFormat)enumVals[tab].value);
    }
}

template <class T>
void
imguiColorWheelForProperty(const char* title, T* property)
{
    // If I am drawing it, it is activated for now.
    const static bool activated = true;
    const T& color = property->get();
    imguiColorWheel("Diffuse color:", &color.x, activated);
}

template <class T>
void
imguiColorWheelForProperties(const char* title, T* properties, uint32_t propertyCount)
{
    // If I am drawing it, it is activated for now.
    static bool activated = true;
    float* color = &properties[0]->get().x;
    // TODO: Needs fixing. Performance drag due to floating point compare and no
    // no invalidation mechanism. Add invalidation mechanism (bool return from imguiColorWheel if value changed).
    imguiColorWheel(title, color, activated);

    for (uint32_t propertyId = 1; propertyId < propertyCount; propertyId++)
    {
        properties[propertyId]->set(properties[0]->get());
    }
}

template <class T, typename S>
void
imguiSliderForPropertyChannel(const char* title, T* property, uint32_t channel, float min, float max, float step)
{
    T& channels = property->get();
    float value = float(channels[channel]);

    if (imguiSlider(title, value, min, max, float(step), enabled))
    {
        // Could do with some validation.
        channels[channelId] = channels;
        property->set(S(value));
    }
}

template <class T>
void
imguiSliderForPropertiesChannel(const char* title, T** properties, uint32_t channel, uint32_t propertyCount, float min, float max, float step)
{
    float* channels = &properties[0]->get().x;
    float value = float(channels[channel]);

    // If we draw it, it is activated.
    static bool activated = true;

    if (imguiSlider(title, value, min, max, float(step), activated))
    {
        // Yay, reference return... Invalidation doesn't work properly for this though.
        properties[0]->get()[channel] = value;

        // For all variants, set.
        for (uint32_t propertyId = 1; propertyId < propertyCount; propertyId++)
            properties[propertyId]->set(properties[0]->get());
    }
}

void 
ApplicationHUD::loadImageForType(InterpretPixelsAsType pixelType)
{
    WCHAR selectedFilePathName[MAX_FILE_PATH_NAME];
    memset(selectedFilePathName, 0, MAX_FILE_PATH_NAME * sizeof(WCHAR));
    WCHAR * filter = L"Direct Draw Surface\0*.DDS\0Targa\0*.TGA\0Tiff\0*.tiff\0Portable Network Graphics\0*.PNG\0Jpg\0*.JPG\0Jpeg\0*.JPEG\0";
    if (Ctr::selectFilenameLoad(selectedFilePathName, filter, L"Open Image"))
    {
        std::wstring inputString(selectedFilePathName);
        std::string  filePathName(inputString.begin(), inputString.end());
        _mrSwizzleApplication->gameTexturesGraph()->loadImage(pixelType, filePathName);
    }
}

void
ApplicationHUD::render(const Ctr::Camera* camera)
{
    int32_t width = _deviceInterface->backbuffer()->width();
    int32_t height = _deviceInterface->backbuffer()->height();

    if (_uiVisible)
    {

        Ctr::Entity* entity = _mrSwizzleApplication->shaderBallEntity();

        std::vector<Ctr::IntProperty*> debugTermProperties;
        std::vector<Ctr::IntProperty*> specularWorkflowProperties;
        std::vector<Ctr::FloatProperty*> roughnessScaleProperties;
        std::vector<Ctr::FloatProperty*> specularIntensityProperties;
        std::vector<Ctr::Vector4fProperty*> userAlbedoProperties;
        std::vector<Ctr::Vector4fProperty*> userRMProperties;
        std::vector<Ctr::Vector4fProperty*> textureScaleOffsetProperties;
        const std::vector<Mesh*>& meshes = entity->meshes();

        specularWorkflowProperties.push_back(_mrSwizzleApplication->specularWorkflowProperty());
        debugTermProperties.push_back(_mrSwizzleApplication->debugTermProperty());

        if (_mrSwizzleApplication->modelVisualizationProperty()->get() == Ctr::MrSwizzleApplication::ShaderBallModel)
        {
            for (auto meshIt = meshes.begin(); meshIt != meshes.end(); meshIt++)
            {

                // ArseImp doesn't load mat or mesh names (or I didn't turn on a preprocessor define to do so).
                // Need to rfind on Albedo for brick.
                // *Note to self, Port ArseImp Fbx implementation to standalone lightweight library*
                //
                Material* currentMaterial = (*meshIt)->material();
                if (currentMaterial->name().rfind("Brick") != std::string::npos)
                {
                    userAlbedoProperties.push_back(currentMaterial->userAlbedoProperty());
                    userRMProperties.push_back(currentMaterial->userRMProperty());
                    textureScaleOffsetProperties.push_back(currentMaterial->textureScaleOffsetProperty());
                }
            }
        }
        else
        {
            for (auto meshIt = meshes.begin(); meshIt != meshes.end(); meshIt++)
            {
                Material* currentMaterial = (*meshIt)->material();
                userAlbedoProperties.push_back(currentMaterial->userAlbedoProperty());
                userRMProperties.push_back(currentMaterial->userRMProperty());

                if (currentMaterial->name().rfind("Brick") != std::string::npos)
                {
                    textureScaleOffsetProperties.push_back(currentMaterial->textureScaleOffsetProperty());
                }
            }
        }

        for (auto meshIt = meshes.begin(); meshIt != meshes.end(); meshIt++)
        {
            Material* currentMaterial = (*meshIt)->material();
            Mesh* currentMesh = (*meshIt);
            debugTermProperties.push_back(currentMaterial->debugTermProperty());
            specularWorkflowProperties.push_back(currentMaterial->specularWorkflowProperty());
            specularIntensityProperties.push_back(currentMaterial->specularIntensityProperty());
            roughnessScaleProperties.push_back(currentMaterial->roughnessScaleProperty());
        }

        Ctr::Vector2i imguiWindowMin(10, 10);
        Ctr::Vector2i imguiWindowMax = imguiWindowMin + Ctr::Vector2i(maxValue(250, width / 5), height - 50);
        Ctr::Region2i imguiWindowBounds(imguiWindowMin, imguiWindowMax);
        _inputState->setHasGUIFocus(imguiWindowBounds.intersects(Ctr::Vector2i(_inputState->_cursorPositionX, _inputState->_cursorPositionY)));

        imguiBeginFrame(_inputState, _inputState->_cursorPositionX, _inputState->_cursorPositionY, _inputState->leftMouseDown() ? IMGUI_MBUT_LEFT : 0 | _inputState->rightMouseDown() ? IMGUI_MBUT_RIGHT : 0, int32_t(_inputState->_z / 25.5), width, height);

        imguiBeginScrollArea("Mr. Swizzle", imguiWindowBounds.minExtent.x, imguiWindowBounds.minExtent.y,
                             imguiWindowBounds.maxExtent.x, imguiWindowBounds.maxExtent.y, &_scrollArea);
        imguiSeparator();

        static bool showPBRTexButtons = true;
        static bool pbrButtonsEnabled = true;
        imguiRegionBorder("PBR", NULL, showPBRTexButtons, pbrButtonsEnabled);
        if (showPBRTexButtons)
        {
            if (imguiButton("Load GameTexture Zip"))
            {
                WCHAR selectedFilePathName[MAX_FILE_PATH_NAME];
                memset(selectedFilePathName, 0, MAX_FILE_PATH_NAME * sizeof(WCHAR));
                WCHAR * filter = L"Archive\0*.zip\0";

                if (selectFilenameLoad(selectedFilePathName, filter))
                {
                    std::wstring inputString(selectedFilePathName);
                    std::string  filePathName(inputString.begin(), inputString.end());

                    _mrSwizzleApplication->gameTexturesGraph()->setupArchivePaths(filePathName);
                }
            }

            if (imguiButton("Load Texture Set"))
            {
                WCHAR selectedFilePathName[MAX_FILE_PATH_NAME];
                memset(selectedFilePathName, 0, MAX_FILE_PATH_NAME * sizeof(WCHAR));
                WCHAR * filter = L"Direct Draw Surface\0*.DDS\0Targa\0*.TGA\0Tiff\0*.tiff\0Portable Network Graphics\0*.PNG\0Jpg\0*.JPG\0Jpeg\0*.JPEG\0";
                if (Ctr::selectFilenameLoad(selectedFilePathName, filter, L"Select albedo image file to specify filename pattern and directory to load from."))
                {
                    std::wstring inputString(selectedFilePathName);
                    std::string  filePathName(inputString.begin(), inputString.end());

                    _mrSwizzleApplication->gameTexturesGraph()->loadTextureSetFromDir(filePathName.c_str());
                }
            }


            if (imguiButton("Save Texture Set"))
            {
                WCHAR selectedFilePathName[MAX_FILE_PATH_NAME];
                memset(selectedFilePathName, 0, MAX_FILE_PATH_NAME * sizeof(WCHAR));
                WCHAR * filter = L"DirectDraw Surfaces(*.dds)\0*.dds\0\0";
                if (Ctr::selectFilenameSave(_mrSwizzleApplication->window()->windowHandle(),
                    selectedFilePathName, filter))
                {
                    std::wstring inputString(selectedFilePathName);
                    std::string  filePathName(inputString.begin(), inputString.end());

                    _mrSwizzleApplication->gameTexturesGraph()->saveImageSet(filePathName.c_str());
                }
            }

            {
                int lastSourceResolution = _mrSwizzleApplication->gameTexturesGraph()->_gameTexturesSrcResolutionProperty->get();
                imguiSelectionSliderForEnumProperty("GameTextures Resolution", _mrSwizzleApplication->gameTexturesGraph()->_gameTexturesSrcResolutionProperty);
                if (lastSourceResolution != _mrSwizzleApplication->gameTexturesGraph()->_gameTexturesSrcResolutionProperty->get())
                {
                    // Update the common resolution, which will cause the sources to be recomputed and
                    // will pick up any changes during evaluation.
                    int sourceSize = 2048;

                    ResolutionType resolutionType = 
                        (ResolutionType)_mrSwizzleApplication->gameTexturesGraph()->_gameTexturesSrcResolutionProperty->get();
                    switch (resolutionType)
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
                    
                    _mrSwizzleApplication->gameTexturesGraph()->_gameTexturesSrcResolutionProperty->set(resolutionType);
                    _mrSwizzleApplication->gameTexturesGraph()->_commonSrcResolutionProperty->set(resolutionType);
                    _mrSwizzleApplication->gameTexturesGraph()->_sourceImageCommonResolutionProperty->set(Ctr::Vector2i(sourceSize, sourceSize));
                }
            }

            {
                int gammaConvert = _mrSwizzleApplication->gameTexturesGraph()->_convertGammaProperty->get();
                imguiSelectionSliderForEnumProperty("Processing Space", _mrSwizzleApplication->gameTexturesGraph()->_convertGammaProperty);
                if (gammaConvert != _mrSwizzleApplication->gameTexturesGraph()->_convertGammaProperty->get())
                {
                    // Update the common resolution, which will cause the sources to be recomputed and
                    // will pick up any changes during evaluation.
                    gammaConvert = _mrSwizzleApplication->gameTexturesGraph()->_convertGammaProperty->get();
                    float gamma = 1.0f;
                    if (gammaConvert == 1)
                        gamma = 2.2f;

                    _mrSwizzleApplication->gameTexturesGraph()->_gammaProperty->set(gamma);
                }
            }

            {
                int lastSourceResolution = _mrSwizzleApplication->gameTexturesGraph()->_commonSrcResolutionProperty->get();
                imguiSelectionSliderForEnumProperty("Common Resolution", _mrSwizzleApplication->gameTexturesGraph()->_commonSrcResolutionProperty);
                if (lastSourceResolution != _mrSwizzleApplication->gameTexturesGraph()->_commonSrcResolutionProperty->get())
                {
                    // Update the common resolution, which will cause the sources to be recomputed and
                    // will pick up any changes during evaluation.
                    int sourceSize = 2048;

                    ResolutionType resolutionType =
                        (ResolutionType)_mrSwizzleApplication->gameTexturesGraph()->_commonSrcResolutionProperty->get();
                    switch (resolutionType)
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
                    _mrSwizzleApplication->gameTexturesGraph()->_sourceImageCommonResolutionProperty->set(Ctr::Vector2i(sourceSize, sourceSize));
                }
            }
        }

        static bool showEnvironmentButtons = false;
        static bool environmentButtonsEnabled = true;
        imguiRegionBorder("Environment", NULL, showEnvironmentButtons, environmentButtonsEnabled);
        if (showEnvironmentButtons)
        {
            imguiIndent();
            if (imguiButton("Load Environment"))
            {
                LOG("Loading an environment");
                WCHAR selectedFilePathName[MAX_FILE_PATH_NAME];
                memset(selectedFilePathName, 0, MAX_FILE_PATH_NAME * sizeof(WCHAR));
                WCHAR * filter = L"All\0*.*\0Text\0*.TXT\0";

                if (selectFilenameLoad(selectedFilePathName, filter))
                {
                    std::wstring inputString(selectedFilePathName);
                    std::string  filePathName(inputString.begin(), inputString.end());

                    _mrSwizzleApplication->loadEnvironment(std::string(filePathName.c_str()));
                }
            }

            if (imguiButton("Save Environment"))
            {
                WCHAR selectedFilePathName[MAX_FILE_PATH_NAME];
                memset(selectedFilePathName, 0, MAX_FILE_PATH_NAME * sizeof(WCHAR));
                WCHAR * filter = L"DirectDraw Surfaces(*.dds)\0*.dds\0\0";
                if (Ctr::selectFilenameSave(_mrSwizzleApplication->window()->windowHandle(),
                    selectedFilePathName, filter))
                {
                    std::wstring inputString(selectedFilePathName);
                    std::string  filePathName(inputString.begin(), inputString.end());

                    _mrSwizzleApplication->saveImages(filePathName.c_str());
                }
            }
            imguiUnindent();
        }


        static bool _showResults = true;
        static bool _resultsEnabled = true;
        imguiRegionBorder("Image Results:", NULL, _showResults, _resultsEnabled);
        if (_showResults)
        {
            imguiIndent();

            imguiSliderForPropertiesChannel("Texture Repeat", &textureScaleOffsetProperties[0], 0, (uint32_t)textureScaleOffsetProperties.size(), 1.0f, 5.0f, 0.05f);
            for (uint32_t propertyId = 0; propertyId < textureScaleOffsetProperties.size(); propertyId++)
            { 
                Vector4f textureScaleOffset = textureScaleOffsetProperties[propertyId]->get();
                textureScaleOffset.y = textureScaleOffset.x;
                textureScaleOffsetProperties[propertyId]->set(textureScaleOffset);
            }

            bool generateMipMaps =
                _mrSwizzleApplication->gameTexturesGraph()->_generateMipMapsProperty->get();
            if (imguiCheck("Generate MipMaps", generateMipMaps, true))
            {
                generateMipMaps = !generateMipMaps;
                _mrSwizzleApplication->gameTexturesGraph()->_generateMipMapsProperty->set(generateMipMaps);
            }

            {
                imguiLabel("Albedo");
                imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_computeAlbedoNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
            }


            {
                imguiLabel("Roughness");
                imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_roughnessRescaleNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
                bool glossToRoughness =
                    _mrSwizzleApplication->gameTexturesGraph()->_convertRoughnessImageNode->imageFunctionProperty()->glossToRoughnessProperty()->get();
                if (imguiCheck("Convert Gloss to Roughness", glossToRoughness, true))
                {
                    glossToRoughness = !glossToRoughness;
                    _mrSwizzleApplication->gameTexturesGraph()->_convertRoughnessImageNode->imageFunctionProperty()->glossToRoughnessProperty()->set(glossToRoughness);
                }
            }


            {
                imguiLabel("Metalness");
                imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_metalRescaleNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
                Vector4f metalMask =
                    _mrSwizzleApplication->gameTexturesGraph()->_extractMetalnessNode->imageFunctionProperty()->metalnessMaskProperty()->get();

                char* metalComponentLabels[] = { "Include R", "Include G", "Include B", "Include A" };
                bool componentChanged = false;
                for (uint32_t componentId = 0; componentId < 4; componentId++)
                {
                    bool invertComponent = metalMask[componentId] > 1e-6f;
                    if (imguiCheck(metalComponentLabels[componentId], invertComponent, true))
                    {
                        invertComponent = !invertComponent;
                        metalMask[componentId] = invertComponent ? 1.0f : 0.0f;
                        componentChanged = true;
                    }
                }
                if (componentChanged)
                {
                    _mrSwizzleApplication->gameTexturesGraph()->_extractMetalnessNode->imageFunctionProperty()->metalnessMaskProperty()->set(metalMask);
                }

            }

            imguiLabel("Tangent Normal");
            imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_convertTangentNormalNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);

            bool swizzleRG = _mrSwizzleApplication->gameTexturesGraph()->_convertTangentNormalNode->imageFunctionProperty()->swizzleRGProperty()->get();
            if (imguiCheck("Swizzle R->G", swizzleRG, true))
            {
                swizzleRG = !swizzleRG;
                _mrSwizzleApplication->gameTexturesGraph()->_convertTangentNormalNode->imageFunctionProperty()->swizzleRGProperty()->set(swizzleRG);
            }


            bool normalizeMips = _mrSwizzleApplication->gameTexturesGraph()->_convertTangentNormalNode->imageFunctionProperty()->normalizeMipsProperty()->get();
            if (imguiCheck("Normalize Mips", normalizeMips, true))
            {
                normalizeMips = !normalizeMips;
                _mrSwizzleApplication->gameTexturesGraph()->_convertTangentNormalNode->imageFunctionProperty()->normalizeMipsProperty()->set(normalizeMips);
            }


            Vector4f invertMask = 
                _mrSwizzleApplication->gameTexturesGraph()->_convertTangentNormalNode->imageFunctionProperty()->inversionMaskProperty()->get();

            char* invertComponentLabels[] = { "Invert R", "Invert G", "Invert B", "Invert A" };
            bool componentChanged = false;
            for (uint32_t componentId = 0; componentId < 4; componentId++)
            {
                bool invertComponent = invertMask[componentId] > 1e-6f;
                if (imguiCheck(invertComponentLabels[componentId], invertComponent, true))
                {
                    invertComponent = !invertComponent;
                    invertMask[componentId] = invertComponent ? 1.0f : 0.0f;
                    componentChanged = true;
                }
            }
            if (componentChanged)
            {
                _mrSwizzleApplication->gameTexturesGraph()->_convertTangentNormalNode->imageFunctionProperty()->inversionMaskProperty()->set(invertMask);
            }
            imguiUnindent();
        }

        static bool swizzleSources = true;
        static bool swizzleSourcesEnabled = true;
        imguiRegionBorder("Swizzle Sources:", NULL, swizzleSources, swizzleSourcesEnabled);
        if (swizzleSources)
        {
            imguiIndent();

            imguiLabel("Albedo");
            imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_albedoImageNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
            if (imguiButton("Load"))
                loadImageForType(AlbedoImage);

            imguiLabel("Normal");
            imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_normalImageNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
            if (imguiButton("Load"))
                loadImageForType(TangentNormalImage);

            imguiLabel("Gloss");
            imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_glossImageNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
            if (imguiButton("Load"))
                loadImageForType(GlossImage);

            imguiLabel("Specular");
            imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_specularImageNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
            if (imguiButton("Load"))
                loadImageForType(SpecularImage);


            imguiLabel("Metal Mask");
            imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_metalMaskImageNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
            if (imguiButton("Load"))
                loadImageForType(MetalMaskImage);

            imguiLabel("AO");
            imguiImage(_mrSwizzleApplication->gameTexturesGraph()->_aoImageNode->textureResultProperty()->get(), 0, 180, 180, ImguiAlign::Center);
            if (imguiButton("Load"))
                loadImageForType(AOImage);

            imguiUnindent();
        }


        imguiRegionBorder("About:", NULL, _showAbout, _aboutEnabled);
        if (_showAbout)
        {
            imguiIndent();
            imguiLabel("Matt Davidson (2015)");
            imguiLabel("Thankyou to:");
            imguiLabel("www.gametextures.com");
            imguiImage(const_cast<ITexture*>(_gameTexture), 0, 256, 128, ImguiAlign::Center);

            imguiUnindent();
        }

        imguiRegionBorder("Rendering:", NULL, _showRendering, _renderingEnabled);
        if (_showRendering)
        {
            imguiIndent();
            imguiPropertySlider("Exposure", _scene->camera()->exposureProperty(), 0.0f, 10.0f, 0.01f);
            imguiPropertySlider("Display Gamma", _scene->camera()->gammaProperty(), 0.0f, 5.0f, 0.01f);

            imguiSelectionSliderForEnumProperty("View Model", _mrSwizzleApplication->modelVisualizationProperty());
            _mrSwizzleApplication->syncVisualization();

            imguiSliderForPropertiesChannel("User Albedo", &userAlbedoProperties[0], 3, (uint32_t)userAlbedoProperties.size(), 0.0f, 1.0f, 0.05f);
            imguiColorWheelForProperties("User Albedo Term", &userAlbedoProperties[0], (uint32_t)userAlbedoProperties.size());

            // These are in whatever working space you have selected.
            imguiSliderForPropertiesChannel("User Gloss/Roughness", &userRMProperties[0], 0, (uint32_t)userRMProperties.size(), 0.0f, 1.0f, 0.05f);
            imguiSliderForPropertiesChannel("User Weight R/G", &userRMProperties[0], 1, (uint32_t)userRMProperties.size(), 0.0f, 1.0f, 0.05f);
            imguiSliderForPropertiesChannel("User Metalness", &userRMProperties[0], 2, (uint32_t)userRMProperties.size(), 0.0f, 1.0f, 0.05f);
            imguiSliderForPropertiesChannel("User Weight M", &userRMProperties[0], 3, (uint32_t)userRMProperties.size(), 0.0f, 1.0f, 0.05f);

            uint32_t meshCount = uint32_t(meshes.size());
            imguiPropertiesSlider("Roughness Scale", &roughnessScaleProperties[0], (uint32_t)roughnessScaleProperties.size(), 0.0f, 1.0f, 0.05f);
            imguiPropertiesSlider("Specular Intensity", &specularIntensityProperties[0], (uint32_t)specularIntensityProperties.size(), 0.0f, 6.0f, 0.05f);
            imguiLabel("Specular Workflow");
            chooseForProperties(&specularWorkflowProperties[0], (uint32_t)specularWorkflowProperties.size());

            imguiLabel("Debug Channel");
            chooseForProperties(&debugTermProperties[0], (uint32_t)debugTermProperties.size());

            imguiUnindent();
        }


        //static float blah[3] = {0.0, 1.0, 1.0};
        //static bool activated = true;
        //imguiColorWheel("Diffuse color:", blah, activated);

        imguiRegionBorder("Filtering:", NULL, _showFiltering, _filteringEnabled);
        if (_showFiltering)
        {
            imguiIndent();

            FloatProperty* inputGammas[2] = { 
                _mrSwizzleApplication->sphereEntity()->mesh(0)->material()->textureGammaProperty(),
                _mrSwizzleApplication->iblSphereEntity()->mesh(0)->material()->textureGammaProperty()
            };

            imguiPropertiesSlider("Input Gamma", &inputGammas[0], 2, 0.0f, 5.0f, 0.01f);
            imguiPropertySlider("Environment Scale", _scene->probes()[0]->environmentScaleProperty(), 0.0f, 10.0f, 0.1f);

            // Lock sample counts for now.
            IntProperty* inputSamples[2] = {
                _scene->probes()[0]->sampleCountProperty(),
                _scene->probes()[0]->samplesPerFrameProperty()
            };

            imguiPropertiesSlider("Sample Count", &inputSamples[0], 2, 0.0f, 2048.0f, 1);
            imguiPropertySlider("Mip Drop", _scene->probes()[0]->mipDropProperty(), 0.0f, _scene->probes()[0]->specularCubeMap()->resource()->mipLevels() - 1.0f, 1);
            imguiPropertySlider("Saturation", _scene->probes()[0]->iblSaturationProperty(), 0.0f, 1.0f, 0.05f);
            //imguiPropertySlider("Contrast", _scene->probes()[0]->iblContrastProperty(), 0.0f, 1.0f, 0.05f);
            imguiPropertySlider("Hue", _scene->probes()[0]->iblHueProperty(), 0.0f, 1.0f, 0.05f);
            imguiPropertySlider("Max Pixel R", _scene->probes()[0]->maxPixelRProperty(), 0.0f, 1000.0f, 1.0f, false);
            imguiPropertySlider("Max Pixel G", _scene->probes()[0]->maxPixelGProperty(), 0.0f, 1000.0f, 1.0f, false);
            imguiPropertySlider("Max Pixel B", _scene->probes()[0]->maxPixelBProperty(), 0.0f, 1000.0f, 1.0f, false);

            imguiSelectionSliderForPixelFormatProperty("Environment Format", _scene->probes()[0]->hdrPixelFormatProperty());
            imguiSelectionSliderForEnumProperty("Source Resolution", _scene->probes()[0]->sourceResolutionProperty());

            imguiSelectionSliderForEnumProperty("Specular Resolution", _scene->probes()[0]->specularResolutionProperty());
            imguiSelectionSliderForEnumProperty("Diffuse Resolution", _scene->probes()[0]->diffuseResolutionProperty());

            imguiUnindent();
        }

        imguiRegionBorder("Brdf:", NULL, _showBrdf, _brdfEnabled);
        if (_showBrdf)
        {
            imguiIndent();

            chooseForProperty(_scene->activeBrdfProperty());

            imguiImage(const_cast<ITexture*>(_scene->activeBrdf()->brdfLut()), 0, 128, 128, ImguiAlign::Center);

            imguiUnindent();
        }

        imguiRegionBorder("Environment:", NULL, _showEnvironment, _environmentEnabled);
        if (_showEnvironment)
        {
            imguiIndent();
            static float _lod = 0.0f;
            static bool _crossCubemapPreview = false;
            if (imguiCube(_scene->probes()[0]->environmentCubeMap(), _lod, _crossCubemapPreview))
            {
                _crossCubemapPreview = !_crossCubemapPreview;
            }

            imguiLabel("Specular IBL:");
            static float _specularEnvLod = 0;
            float maxSpecularMipLevels = _scene->probes()[0]->specularCubeMap()->resource()->mipLevels() - 1.0f - _scene->probes()[0]->mipDropProperty()->get();
            if (_specularEnvLod > maxSpecularMipLevels)
                maxSpecularMipLevels = maxSpecularMipLevels;
            imguiSlider("IBL LOD", _specularEnvLod, 0.0f, maxSpecularMipLevels, 0.15f);
        
            static bool _specularCrossCubemapPreview = false;
            if (imguiCube(_scene->probes()[0]->specularCubeMap(), _specularEnvLod, _specularCrossCubemapPreview))
            {
                _specularCrossCubemapPreview = !_specularCrossCubemapPreview;
            }

            imguiLabel("Irradiance IBL:");
            static float _diffuseEnvLod = 0.0f;
            static bool _diffuseCrossCubemapPreview = false;
            if (imguiCube(_scene->probes()[0]->diffuseCubeMap(), _diffuseEnvLod, _diffuseCrossCubemapPreview))
            {
                _diffuseCrossCubemapPreview = !_diffuseCrossCubemapPreview;
            }
            imguiUnindent();
        }
        imguiEndScrollArea();
        imguiEndFrame();
    }

    RenderHUD::render(camera);
}



}
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
#include <direct.h>

namespace
{
void
setupStartupPath()
{
    WCHAR pathName[MAX_PATH];
    memset(pathName, 0, sizeof(CHAR) * MAX_PATH);
    HMODULE moduleHandle = NULL;

    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPWSTR)&setupStartupPath,
        &moduleHandle))
    {
        return;
    }
    if (!GetModuleFileName(moduleHandle, pathName, sizeof(pathName)))
    {
        return;
    }

    std::wstring modulePathFileName(pathName);
    std::string modulePathName(modulePathFileName.begin(), modulePathFileName.begin() + modulePathFileName.rfind(L'\\'));
    LOG("Mr.Swizzle: " << modulePathName);

    std::string sandboxPathName(modulePathName.begin(), modulePathName.begin()+modulePathName.rfind("\\"));
    _chdir(sandboxPathName.c_str());
}
}

int main(int argc, char* argv[])
{
    ApplicationHandle applicationInstance = nullptr;

    setupStartupPath();
#if _WIN32
    applicationInstance = GetModuleHandle(nullptr);
#else 
    LOG ("Platform is not yet implemented");
    assert(0)
#endif

    assert (applicationInstance);
    std::unique_ptr<Ctr::MrSwizzleApplication> application;
    application.reset(new Ctr::MrSwizzleApplication(applicationInstance));
    bool encounteredError = false;

    if (application)
    {
        // Load parameters from data/parameters.xml
        if (application->loadParameters())
        {
            // Parse options and override parameters.
            if (application->parseOptions(argc, argv))
            {
                try
                {
                    // Initialization failure will throw std::runtime_error on failure.
                    application->initialize();

                    // Run failure will throw std::runtime_error on error.
                    application->run();
                }
                catch (const std::runtime_error& error)
                {
                    // Errored out, attempt to exit.
                    LOG_CRITICAL("Something terrible happened: " << error.what());
                    return 0;
                }
                // Save out application settings
                application->saveParameters();
            }
        }
        // Destroy the application.
        LOG("Destroying the application");
        application.reset();
    }
    LOG("Mr.Swizzle has completed.");
    return 0;
}

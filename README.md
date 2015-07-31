MrSwizzle
--------------

I am still writing documentation for this project.
This is not really fit to be public, but I'd like some people to play around with it... Will most likely not work for some workflows until after Siggraph.

MrSwizzle is a quick tool that was started to quickly convert gametextures.com and other formats to a common PBR representation.
This tool runs both as a headless commandline tool and interactive gui tool.
I am interested in providing support for Unreal and Unity, but I don't use these engines myself.
Please let me know if this is / would be useful to you and I am happy to add any required extra operations or pipeline additions.

![MrSwizzleCover1](https://github.com/derkreature/MrSwizzle/blob/master/images/screenshot0.jpg)
![MrSwizzleCover2](https://github.com/derkreature/MrSwizzle/blob/master/images/screenshot1.jpg)


About
--------------
MrSwizzle is provided under the MIT License(MIT)
Copyright(c) 2015 Matt Davidson.
Please see the LICENSE file for full details.

Feel free to contact me with questions, corrections or ideas.
https://github.com/derkreature/
http://www.derkreature.com/

Quick download link:
https://github.com/derkreature/MrSwizzle/archive/master.zip

Using MrSwizzle on the command line
--------------

All commandline options are optional.
Valid arguments to MrSwizzle include:

--help         Shows this list of commands.
--headless     Runs the tool Headless, I.e., without a window or GUI. This is also the only way to run the tool and have output saved from the commandline.
--gtarchive    Specify a gametextures.com Archive. E.g., "c:/Metal_GalvanizedTrashy_PBR.zip"
--dirts        Specify a Texture Set using the Albedo Path File Name for the texture set.
--envsrc       Specify environment map filename source and pathname (Cross cube or Lat Long dds, exr, or hdr).
--envout       Environment Output Path and FileName (must be a DDS filename, all outputs are based on this filename and path).
--pbrout       Specify the PBR output pattern filename. (Should be a DDS filename if you wish to preserve mips). Can be any freeimage 
--alb          Albedo Filename Pattern. E.g., if "alb" would form the albedo path redbrick_alb.tga or redbrick_2k_alb.tga for the 2k albedo image in a gametextures archive.
--norm         Tangent Normal Filename Pattern. E.g. E.g., if "nrm" would form the albedo path redbrick_nrm.tga or redbrick_2k_nrm.tga for the 2k tangent normal image in a gametextures archive.
--gloss        Gloss Filename Pattern. E.g. E.g., if "g" would form the gloss path redbrick_g.tga or redbrick_2k_g.tga for the 2k tangent normal image in a gametextures archive.
--metal        Metal Mask Filename Pattern", false, "mask");
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



Using MrSwizzle in its GUI
--------------

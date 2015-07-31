MrSwizzle
--------------

MrSwizzle is a small command line and gui tool that was started to quickly convert gametextures.com and other formats to a common PBR representation.
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

- [--help]         Shows this list of commands.
- [--headless]     Runs the tool Headless, I.e., without a window or GUI. This is also the only way to run the tool and have output saved from the commandline.
- [--gtarchive]    Specify a gametextures.com Archive. E.g., "c:/Metal_GalvanizedTrashy_PBR.zip"
- [--dirts]        Specify a Texture Set using the Albedo Path File Name for the texture set.
- [--pbrout]       Specify the PBR output pattern filename. (Should be a DDS filename if you wish to preserve mips). Can be any freeimage 
- [--envsrc]       Specify environment map filename source and pathname (Cross cube or Lat Long dds, exr, or hdr).
- [--envout]       Environment Output Path and FileName (must be a DDS filename, all outputs are based on this filename and path).
- [--alb]          Albedo Filename Pattern. E.g., if "alb" would form the albedo path redbrick_alb.tga or redbrick_2k_alb.tga for the 2k albedo image in a gametextures archive.
- [--norm]         Tangent Normal Filename Pattern. E.g., if "nrm" would form the albedo path redbrick_nrm.tga or redbrick_2k_nrm.tga for the 2k tangent normal image in a gametextures archive.
- [--gloss]        Gloss Filename Pattern. E.g., if "g" would form the gloss path redbrick_g.tga or redbrick_2k_g.tga for the 2k tangent normal image in a gametextures archive.
- [--metal]        Metal Mask Filename Pattern. E.g., if "metal" would form the metal path redbrick_metal.tga or redbrick_2k_metal.tga for the 2k metal image in a gametextures archive.
- [--s]            Specular Color Filename Pattern. E.g., if "specular" would form the specular path redbrick_specular.tga or redbrick_2k_specular.tga for the 2k specular image in a gametextures archive.
- [--ao]           Ambient Occlusion Filename Pattern. E.g., if "ao" would form the ambient occlusion path redbrick_ao.tga or redbrick_2k_ao.tga for the 2k metal image in a gametextures archive.
- [--height]       Height Filename Pattern. E.g., if "height" would form height path redbrick_height.tga or redbrick_2k_height.tga for the 2k metal image in a gametextures archive.
- [--rmc]          Roughness - Metalness - Caviety Filename Pattern. (Output file).
- [--gtres]    	   Game Textures Resolution. Valid values are 256, 512, 1024, 2048. Not used if loading from an image set on disk.
- [--commonres]    Common Resolution. All input images are rescaled to this common resolution for image processing operations. 
- [--umr]          Use Metal Mask R. Boolean that specifies whether to use the Red channel when generating the metalness mask.
- [--umg]          Use Metal Mask G. Boolean that specifies whether to use the Green channel when generating the metalness mask.
- [--umb]          Use Metal Mask B. Boolean that specifies whether to use the Blue channel when generating the metalness mask.
- [--uma]          Use Metal Mask B. Boolean that specifies whether to use the Blue channel when generating the metalness mask.
- [--gm]           Generate mips, on or off. Boolean that specifies whether to generate mip maps or not.
- [--nm]           Normalize Normal Map Mips. Boolean that specifies whether to renormalize normalmap mips.
- [--gentok]       Boolean indicating whether to generate Toksvig maps from unnormalized normal map mips.
- [--cps]          String indicating whether to process color pixels in "Linear" or "Gamma" space.
- [--uvscale]      Float specifying the texture repeat scale on the visualization mesh.
    

Using MrSwizzle in its GUI
--------------

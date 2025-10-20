#  ANIM

From Just Solve the File Format Problem

Jump to: navigation, search

[File Format](/wiki/File_Formats "File Formats")  
---  
Name | ANIM  
Ontology  | 

  * [Electronic File Formats](/wiki/Electronic_File_Formats "Electronic File Formats")
    * [Animation](/wiki/Animation "Animation")
      * **ANIM**

  
[Extension(s)](/wiki/Filename_extension "Filename extension") | `.anim`, `.sndanim`, others  
[Wikidata ID](/wiki/Wikidata_identifier "Wikidata identifier") | [Q4652973](https://www.wikidata.org/wiki/Q4652973)  
Released | 1988  
  
    _Distinct from[DeluxePaint Animation](/wiki/DeluxePaint_Animation "DeluxePaint Animation") (Anim or ANM) files._

**ANIM** is an animated raster graphics format that was widely used on Amiga
computers.

It uses the [IFF](/wiki/IFF "IFF") container format. The first frame is
usually (more or less) in the form of an embedded [ILBM](/wiki/ILBM "ILBM")
file. The other frames use one of a number of available schemes for storing
the differences from a previous frame.

See also:

  * [AnimBrush](/wiki/AnimBrush "AnimBrush")
  * [ILBM](/wiki/ILBM "ILBM")

## Contents

  * 1 Format details
    * 1.1 Frame operations
    * 1.2 Audio
  * 2 Identification
  * 3 Specifications
  * 4 Software
  * 5 Sample files
  * 6 Links

  
---  
  
##  Format details

###  Frame operations

Each frame (except possibly the first frame) has an `ANHD` chunk with an
"operation" field, which tells how that frame's pixel data is structured,
compressed, etc. Usually, for a given ANIM file, all frames with a nonzero
operation use the same operation.

Only operations 0, 5, and 7 are reasonably common.

Known operations:

Operation  |  Name  |  Description  |  Remarks   
---|---|---|---  
0  |  |  direct ILBM  |  Frame uses the [ILBM](/wiki/ILBM "ILBM") image format. The actual compression method (usually [PackBits](/wiki/PackBits "PackBits")) is given by a field in the frame's `BMHD` chunk.   
1  |  ANIM-1, ANIM1  |  XOR ILBM  |  Rarely supported.   
2  |  ANIM-2, ANIM2  |  long delta  |  Rarely supported.   
3  |  ANIM-3, ANIM3  |  short delta  |   
4  |  ANIM-4, ANIM4  |  generalized delta  |  Rarely supported.   
5  |  ANIM-5, ANIM5  |  byte vertical delta  |  The most common frame format.   
6  |  ANIM-6, ANIM6  |  stereo byte vertical delta  |  Rarely supported.   
7  |  ANIM-7, ANIM7  |  generalized vertical delta, separate data  |   
8  |  ANIM-8, ANIM8  |  generalized vertical delta, inline data  |   
74  |  ANIM-J, ANIM-74  |  |  Eric Graham's format. Used by Sculpt 3D, etc. More than a frame format, ANIM-J is an extension that supports more sophisticated playback.   
100  |  ANIM32  |  long vertical  |  Reportedly used by Scala Multimedia/MM400 and InfoChannel/IC500.   
101  |  ANIM16  |  short vertical  |  Reportedly used by Scala Multimedia/MM400 and InfoChannel/IC500.   
108  |  ANIM-l  |  |  (That's a lowercase _L_ \- ASCII 108.) Reportedly by Eric Graham.   
  
###  Audio

There are at least a couple of extensions that add audio to an ANIM file.

  * .sndanim: Refer to AnimFX's documentation of the `SXHD` and `SBDY` chunks. 
  * An extension based on [8SVX](/wiki/8-Bit_Sampled_Voice "8-Bit Sampled Voice"). 

##  Identification

Files begin with bytes `'F' 'O' 'R' 'M' ?? ?? ?? ?? 'A' 'N' 'I' 'M'`.

##  Specifications

  * [ANIM IFF CEL Animations](https://wiki.amigaos.net/wiki/ANIM_IFF_CEL_Animations), from the AmigaOS Documentation Wiki 
  * <http://www.textfiles.com/programming/FORMATS/anim7.txt>
  * [IFF.TXT](https://www.fileformat.info/format/iff/spec/7866a9f0e53c42309af667c5da3bd426/view.htm) \- Section named "Cel animation form" 
  * [AnimFX](http://aminet.net/package/gfx/show/AnimFX) → ANIM.guide 

##  Software

  * [iffanimplay](https://github.com/murkymark/iffanimplay)
  * [AnimPlayer](https://blitterstudio.com/animplayer/)
  * [XAnim](http://xanim.polter.net/)
  * [cvtmovie](http://www.etwright.org/cghist/cvtmovie.html) \- ANIM-J information and extractor 
  * [FFmpeg](/wiki/FFmpeg "FFmpeg")
  * [iff-convert](http://www.boomerangsworld.de/cms/tools/iff-convert.html)

##  Sample files

Various:

  * <http://aminet.net/pix/anim>
  * <http://samples.mplayerhq.hu/anim/>
  * [dexvert samples — video/iffANIM](http://sembiance.com/fileFormatSamples/video/iffANIM)

By type:

  * ANIM3: [[1]](http://aminet.net/package/pix/anim/SpacePatrol), [[2]](http://aminet.net/package/pix/anim/ZEUS)
  * ANIM7: [[3]](http://aminet.net/package/pix/anim/FarmersAnim), [[4]](http://aminet.net/package/pix/anim/vfxanim), [[5]](http://aminet.net/package/pix/anim/wavingrose)
  * ANIM8: [[6]](http://aminet.net/package/pix/anim/asteroids), [[7]](http://aminet.net/package/pix/anim/Warp)
  * ANIM-J: [[8]](http://aminet.net/package/pix/anim/Spigot), [[9]](http://aminet.net/package/pix/anim/Doctor_A)
  * ANIM32: [[10]](http://aminet.net/package/demo/aga/MSV_TwistedMinds) (241 MB) 
  * ANIM-l: [[11]](http://aminet.net/package/pix/anim/ElGato)
  * AnimFX audio: [[12]](http://aminet.net/package/pix/anim/Wz), [[13]](http://aminet.net/pix/anim/City.lha)
  * 8SVX audio: [[14]](http://aminet.net/package/pix/anim/gressklipperma)

See also [AnimBrush#Sample files](/wiki/AnimBrush#Sample_files "AnimBrush").

##  Links

  * [Wikipedia article](http://en.wikipedia.org/wiki/ANIM "wikipedia:ANIM")

Retrieved from
"[http://fileformats.archiveteam.org/index.php?title=ANIM&oldid=48515](http://fileformats.archiveteam.org/index.php?title=ANIM&oldid=48515)"

[Categories](/wiki/Special:Categories "Special:Categories"):

  * [File Formats](/wiki/Category:File_Formats "Category:File Formats")
  * [Electronic File Formats](/wiki/Category:Electronic_File_Formats "Category:Electronic File Formats")
  * [Animation](/wiki/Category:Animation "Category:Animation")
  * [File formats with extension .anim](/wiki/Category:File_formats_with_extension_.anim "Category:File formats with extension .anim")
  * [File formats with extension .sndanim](/wiki/Category:File_formats_with_extension_.sndanim "Category:File formats with extension .sndanim")
  * [IFF based file formats](/wiki/Category:IFF_based_file_formats "Category:IFF based file formats")
  * [Amiga](/wiki/Category:Amiga "Category:Amiga")
  * [Video](/wiki/Category:Video "Category:Video")

Hidden category:

  * [FormatInfo without mimetypes](/wiki/Category:FormatInfo_without_mimetypes "Category:FormatInfo without mimetypes")

##### Personal tools

  * [Log in / create account](/index.php?title=Special:UserLogin&returnto=ANIM "You are encouraged to log in; however, it is not mandatory \[o\]")

##### Namespaces

  * [Page](/wiki/ANIM "View the content page \[c\]")
  * [Discussion](/index.php?title=Talk:ANIM&action=edit&redlink=1 "Discussion about the content page \[t\]")

####

##### Variants

##### Views

  * [Read](/wiki/ANIM)
  * [View source](/index.php?title=ANIM&action=edit "This page is protected.
You can view its source \[e\]")

  * [View history](/index.php?title=ANIM&action=history "Past revisions of this page \[h\]")

##### Actions

##### Search

[](/wiki/Main_Page "Visit the main page")

##### Navigation

  * [Main page](/wiki/Main_Page "Visit the main page \[z\]")
  * [File formats](/wiki/File_Formats)
  * [Formats by extension](/wiki/Category:File_formats_by_extension)
  * [Still more extensions](/wiki/Category:File_Format_Extension)
  * [Software](/wiki/Software)
  * [Glossary](/wiki/Glossary)
  * [Library](/wiki/Library)
  * [Sources](/wiki/Sources)
  * [Categories](/wiki/Category:Top_Level_Categories)
  * [Community portal](/wiki/Just_Solve_the_File_Format_Problem:Community_portal "About the project, what you can do, where to find things")
  * [Recent changes](/wiki/Special:RecentChanges "A list of recent changes in the wiki \[r\]")
  * [Random page](/wiki/Special:Random "Load a random page \[x\]")

##### Toolbox

  * [What links here](/wiki/Special:WhatLinksHere/ANIM "A list of all wiki pages that link here \[j\]")
  * [Related changes](/wiki/Special:RecentChangesLinked/ANIM "Recent changes in pages linked from this page \[k\]")
  * [Special pages](/wiki/Special:SpecialPages "A list of all special pages \[q\]")
  * [Printable version](/index.php?title=ANIM&printable=yes)
  * [Permanent link](/index.php?title=ANIM&oldid=48515 "Permanent link to this revision of the page")

  * This page was last modified on 28 April 2024, at 15:35.
  * This page has been accessed 162,948 times.
  * Content is available under [Creative Commons 0](http://creativecommons.org/publicdomain/zero/1.0/).

  * [Privacy policy](/wiki/Just_Solve_the_File_Format_Problem:Privacy_policy "Just Solve the File Format Problem:Privacy policy")
  * [About Just Solve the File Format Problem](/wiki/Just_Solve_the_File_Format_Problem:About "Just Solve the File Format Problem:About")
  * [Disclaimers](/wiki/Just_Solve_the_File_Format_Problem:General_disclaimer "Just Solve the File Format Problem:General disclaimer")

  * [![Creative Commons 0](http://www.mediawiki.org/w/skins/common/images/cc-0.png)](http://creativecommons.org/publicdomain/zero/1.0/)
  * [![Powered by MediaWiki](/skins/common/images/poweredby_mediawiki_88x31.png)](//www.mediawiki.org/)


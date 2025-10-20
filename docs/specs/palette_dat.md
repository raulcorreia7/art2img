#  DAT (Duke Nukem 3D)

From Just Solve the File Format Problem

Jump to: navigation, search

[File Format](/wiki/File_Formats "File Formats")  
---  
Name | DAT (Duke Nukem 3D)  
Ontology  | 

  * [Electronic File Formats](/wiki/Electronic_File_Formats "Electronic File Formats")
    * [Graphics](/wiki/Graphics "Graphics")
      * **DAT (Duke Nukem 3D)**

  
[Extension(s)](/wiki/Filename_extension "Filename extension") | `.dat`  
  
The **DAT** format is a palette format used by _Duke Nukem 3D_ (a [Build
Engine](/wiki/Category:Build_Engine_formats "Category:Build Engine formats")
game). Typically, these files are named `PALETTE.DAT`.

The format contains three chunks:

  * A **palette** of 256 colors, based on the _VGA 262,144 color palette_. Colors are stored as a tuple of 3 bytes in sequence, comprising the red, blue and green component. Each component's value is in the range of 0-63. 
  * **Shading Lookup Table** which is used to reference the colors in the palette at different brightnesses, from full-bright to black. 
  * **Translucent Lookup Table** , which given any two colors, provides reference to the closest matching color in the palette representing the blend of those two colors. 

## Contents

  * 1 File signature
  * 2 Specification
  * 3 Sample files
  * 4 See also

  
---  
  
##  File signature

This format does not contain a file signature/magic bytes.

##  Specification

Ken Silverman, the original author of the [Build
Engine](/wiki/Category:Build_Engine_formats "Category:Build Engine formats"),
provided the following documentation of the format:

    
    
    http://advsys.net/ken/palette.txt
    
    Here's some pseudo-C code which explains how to load the PALETTE.DAT file:
    
    	 char palette[768], palookup[numpalookups][256], transluc[256][256];
    	 short numpalookups;
     
    	 fil = open("PALETTE.DAT",...);
    	 read(fil,palette,768);
    	 read(fil,&numpalookups,2);
    	 read(fil,palookup,numpalookups*256);
    	 read(fil,transluc,65536);
    	 close(fil);
     
    PALETTE: This 768 byte array is exactly the palette you want.  The format is:
    	 Red0, Green0, Blue0, Red1, Green1, Blue1, ..., Blue255
    The colors are based on the VGA 262,144 color palette.  The values range from
    0-63, so if you want to convert it to a windows palette you will have to
    multiply each byte by 4.
     
    NUMPALOOKUPS: The number of shading tables used.  Usually this number is 32,
    but 16 or 64 have also been used.  Each of the 256 colors of the VGA palette
    can take on any of "numpalookups" number of shades.
     
    PALOOKUP: The shading table. If numpalookups = 32, then this table is:
    (32 shades) * (256 colors) = 8192 bytes (8K).  The shade tables are often
    made to go from normal brightness (shade #0) down to pitch black (shade #31)
    So the first 256 bytes of the table would be for shade #0, etc...
     
    TRANSLUC: 64K translucent lookup table.  Given any 2 colors of the palette,
    this lookup table gives the best match of the 2 colors when mixed together.
    
    Here's a funny story: I noticed that Duke3D's PALETTE.DAT file is 8K longer
    than it should be.  Any PALETTE.DAT file with 32 shades and translucent table
    should be 74,498 bytes.  Duke3D's palette is 82,690 bytes, but it only has 32
    shades!  The reason is that at one time, Duke3D had 64 shades in their
    "palookup" table.  Then when we noticed that this extra memory overhead
    slowed down the frame rate of the game noticably, it was converted back to
    32 shades.  The problem is that my palette conversion program never
    truncated off the end of the file.  So the last 8K of Duke3D's PALETTE.DAT
    is the last 8K of a translucent table that was based on an older version of
    their palette.
    

##  Sample files

  * [Boot Disc Issue 01](https://archive.org/details/cdrom-boot-disc-01) → CONTENT/DEMOS/GAMES/DUKE3D/DUKE3D.GRP ([GRP](/wiki/GRP_\(Duke_Nukem_3D\) "GRP \(Duke Nukem 3D\)") format) → PALETTE.DAT (from the Duke3D demo) 

##  See also

  * [_Duke Nukem 3D Palette Format_ on the _DOS Game Modding Wiki_](https://moddingwiki.shikadi.net/wiki/Duke_Nukem_3D_Palette_Format)

Retrieved from
"[http://fileformats.archiveteam.org/index.php?title=DAT_(Duke_Nukem_3D)&oldid=44596](http://fileformats.archiveteam.org/index.php?title=DAT_\(Duke_Nukem_3D\)&oldid=44596)"

[Categories](/wiki/Special:Categories "Special:Categories"):

  * [File Formats](/wiki/Category:File_Formats "Category:File Formats")
  * [Electronic File Formats](/wiki/Category:Electronic_File_Formats "Category:Electronic File Formats")
  * [Graphics](/wiki/Category:Graphics "Category:Graphics")
  * [File formats with extension .dat](/wiki/Category:File_formats_with_extension_.dat "Category:File formats with extension .dat")
  * [Build Engine formats](/wiki/Category:Build_Engine_formats "Category:Build Engine formats")
  * [Palettes](/wiki/Category:Palettes "Category:Palettes")
  * [Game data files](/wiki/Category:Game_data_files "Category:Game data files")

Hidden category:

  * [FormatInfo without mimetypes](/wiki/Category:FormatInfo_without_mimetypes "Category:FormatInfo without mimetypes")

##### Personal tools

  * [Log in / create account](/index.php?title=Special:UserLogin&returnto=DAT+%28Duke+Nukem+3D%29 "You are encouraged to log in; however, it is not mandatory \[o\]")

##### Namespaces

  * [Page](/wiki/DAT_\(Duke_Nukem_3D\) "View the content page \[c\]")
  * [Discussion](/index.php?title=Talk:DAT_\(Duke_Nukem_3D\)&action=edit&redlink=1 "Discussion about the content page \[t\]")

####

##### Variants

##### Views

  * [Read](/wiki/DAT_\(Duke_Nukem_3D\))
  * [View source](/index.php?title=DAT_\(Duke_Nukem_3D\)&action=edit "This page is protected.
You can view its source \[e\]")

  * [View history](/index.php?title=DAT_\(Duke_Nukem_3D\)&action=history "Past revisions of this page \[h\]")

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

  * [What links here](/wiki/Special:WhatLinksHere/DAT_\(Duke_Nukem_3D\) "A list of all wiki pages that link here \[j\]")
  * [Related changes](/wiki/Special:RecentChangesLinked/DAT_\(Duke_Nukem_3D\) "Recent changes in pages linked from this page \[k\]")
  * [Special pages](/wiki/Special:SpecialPages "A list of all special pages \[q\]")
  * [Printable version](/index.php?title=DAT_\(Duke_Nukem_3D\)&printable=yes)
  * [Permanent link](/index.php?title=DAT_\(Duke_Nukem_3D\)&oldid=44596 "Permanent link to this revision of the page")

  * This page was last modified on 27 June 2023, at 20:11.
  * This page has been accessed 9,642 times.
  * Content is available under [Creative Commons 0](http://creativecommons.org/publicdomain/zero/1.0/).

  * [Privacy policy](/wiki/Just_Solve_the_File_Format_Problem:Privacy_policy "Just Solve the File Format Problem:Privacy policy")
  * [About Just Solve the File Format Problem](/wiki/Just_Solve_the_File_Format_Problem:About "Just Solve the File Format Problem:About")
  * [Disclaimers](/wiki/Just_Solve_the_File_Format_Problem:General_disclaimer "Just Solve the File Format Problem:General disclaimer")

  * [![Creative Commons 0](http://www.mediawiki.org/w/skins/common/images/cc-0.png)](http://creativecommons.org/publicdomain/zero/1.0/)
  * [![Powered by MediaWiki](/skins/common/images/poweredby_mediawiki_88x31.png)](//www.mediawiki.org/)


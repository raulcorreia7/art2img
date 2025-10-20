# ART Format (Build)

From ModdingWiki

Jump to navigation Jump to search

**ART Format (Build)**

_There is no image of a tileset in this format —[upload
one](https://moddingwiki.shikadi.net/w/index.php?title=Special:Upload&wpDestFile=ART_Format_\(Build\).png)!_

Format type| Tileset  
---|---  
Hardware| VGA  
Max tile count| 231 \- 1  
Palette| Shared VGA  
Tile names?| No  
Minimum tile size (pixels)| 0×0  
Maximum tile size (pixels)| 32767×32767  
Plane count| 1  
Plane arrangement| [Linear VGA](/wiki/Linear_VGA "Linear VGA")  
Transparent pixels?| No  
Hitmap pixels?| No  
Metadata?| None  
Supports sub-tilesets?| No  
Compressed tiles?| No  
Hidden data?| No  
Games| [Blood](/wiki/Blood "Blood") [Duke Nukem 3D](/wiki/Duke_Nukem_3D "Duke
Nukem 3D") [Shadow Warrior](/wiki/Shadow_Warrior "Shadow Warrior") [Redneck
Rampage](/w/index.php?title=Redneck_Rampage&action=edit&redlink=1 "Redneck
Rampage \(page does not exist\)") [Witchaven](/wiki/Witchaven "Witchaven")
[Witchaven II](/wiki/Witchaven_II "Witchaven II")  
  
The **ART format** is used by Ken Silverman's Build engine to store game
textures and sprites. It contains a number of individual graphics called
tiles.

## Contents

  * 1 File Names and Tile Numbers
  * 2 Header
  * 3 Tile Attributes
  * 4 Pixel Data
  * 5 Source

## File Names and Tile Numbers

According to Ken Silverman's documentation, ART files can be named anything,
as long as they end in three digits and .ART, but for most games they are
named TILES###.ART. The Build engine games will load these in order starting
from 000 until they can't find any more. Every tile in the game has a
numerical index, and each ART file for a particular game holds a range of
these. For example, TILES000.ART may hold tiles 0 to 255, and TILES001.ART
would hold tiles 256-511. The range of indexes held by a particular file are
indicated in the header. The tiles are normally stored sequentially
(TILES000.ART holding the first set, and each higher numbered file continuing
where the previous left off) with a consistent number of tiles per file, but
it is up to the particular game's developers.

## Header

Data Type | Name | Description   
---|---|---  
[INT32LE](/wiki/INT32LE "INT32LE") | artversion  | version number, should be 1   
[INT32LE](/wiki/INT32LE "INT32LE") | numtiles  | number of tiles, unused. The number of tiles can be determined by the localtilestart and localtileend fields.   
[INT32LE](/wiki/INT32LE "INT32LE") | localtilestart  | number of first tile in this file   
[INT32LE](/wiki/INT32LE "INT32LE") | localtileend  | number of last tile in this file.   
[INT16LE](/wiki/INT16LE "INT16LE")[localtileend-localtilestart + 1]  | tilesizx  | array of the x-dimensions of all of the tiles in the file   
[INT16LE](/wiki/INT16LE "INT16LE")[localtileend-localtilestart + 1]  | tilesizy  | array of the y-dimensions of all of the tiles in the file   
[INT32LE](/wiki/INT32LE "INT32LE")[localtileend-localtilestart + 1]  | picanm  | array of attributes for all the tiles   
  
## Tile Attributes

Entries in the picanm array store several properties for each tile:

bits 31-28  | bits 27-24 (4 bit unsigned integer)  | bits 23-16 (8 bit signed integer)  | bits 15-8 (8 bit signed integer)  | bits 7 and 6 (2 bit enumeration)  | bits 5-0 (6-bit unsigned integer)   
---|---|---|---|---|---  
unused?  | Animation speed  | Y-center offset  | X-center offset  | Animation type: 

  * 00 = no animation
  * 01 = oscillation animation
  * 10 = animate forward
  * 11 = animate backward

| Number of frames  
  
## Pixel Data

The pixels are stored as bytes, corresponding to indexes in the palette stored
in PALETTE.DAT. The pixels in each tile are stored columnwise, starting from
the top-left. For a 4x4 tile, the offsets for each pixel would be arranged
like this:

0 | 4 | 8 | 12   
---|---|---|---  
1 | 5 | 9 | 13   
2 | 6 | 10 | 14   
3 | 7 | 11 | 15   
  
## Source

  * This information comes from `[BUILDINF.TXT](https://github.com/jonof/jfbuild/blob/master/doc/buildinf.txt)` in the [Build source](http://advsys.net/ken/buildsrc/default.htm) files, written by Ken Silverman.

Retrieved from
"[https://moddingwiki.shikadi.net/w/index.php?title=ART_Format_(Build)&oldid=8779](https://moddingwiki.shikadi.net/w/index.php?title=ART_Format_\(Build\)&oldid=8779)"

[Categories](/wiki/Special:Categories "Special:Categories"):

  * [All file formats](/wiki/Category:All_file_formats "Category:All file formats")
  * [All tileset formats](/wiki/Category:All_tileset_formats "Category:All tileset formats")
  * [VGA tilesets](/wiki/Category:VGA_tilesets "Category:VGA tilesets")
  * [Nameless tilesets](/wiki/Category:Nameless_tilesets "Category:Nameless tilesets")
  * [Shallow tilesets](/wiki/Category:Shallow_tilesets "Category:Shallow tilesets")
  * [Uncompressed tilesets](/wiki/Category:Uncompressed_tilesets "Category:Uncompressed tilesets")
  * [Dense tilesets](/wiki/Category:Dense_tilesets "Category:Dense tilesets")
  * [Need pictures](/w/index.php?title=Category:Need_pictures&action=edit&redlink=1 "Category:Need pictures \(page does not exist\)")
  * [Blood](/wiki/Category:Blood "Category:Blood")
  * [Duke Nukem 3D](/wiki/Category:Duke_Nukem_3D "Category:Duke Nukem 3D")
  * [Shadow Warrior](/wiki/Category:Shadow_Warrior "Category:Shadow Warrior")
  * [Redneck Rampage](/w/index.php?title=Category:Redneck_Rampage&action=edit&redlink=1 "Category:Redneck Rampage \(page does not exist\)")
  * [Witchaven](/wiki/Category:Witchaven "Category:Witchaven")
  * [Witchaven II](/wiki/Category:Witchaven_II "Category:Witchaven II")
  * [Build engine](/wiki/Category:Build_engine "Category:Build engine")

## Navigation menu

###  Personal tools

  * [Log in](/w/index.php?title=Special:UserLogin&returnto=ART+Format+%28Build%29 "You are encouraged to log in; however, it is not mandatory \[o\]")
  * [Request account](/wiki/Special:RequestAccount "You are encouraged to create an account and log in; however, it is not mandatory")

###  Namespaces

  * [Page](/wiki/ART_Format_\(Build\) "View the content page \[c\]")
  * [Discussion](/w/index.php?title=Talk:ART_Format_\(Build\)&action=edit&redlink=1 "Discussion about the content page \(page does not exist\) \[t\]")

British English

###  Views

  * [Read](/wiki/ART_Format_\(Build\))
  * [View source](/w/index.php?title=ART_Format_\(Build\)&action=edit "This page is protected.
You can view its source \[e\]")

  * [View history](/w/index.php?title=ART_Format_\(Build\)&action=history "Past revisions of this page \[h\]")

More

###  Search

[](/wiki/Main_Page "Visit the main page")

###  Navigation

  * [Main Page](/wiki/Main_Page "Visit the main page \[z\]")

###  Games

  * [By title](/wiki/Category:Game_Intro_Page)
  * [By company](/wiki/Category:Game_Company)
  * [By genre](/wiki/Category:Game_Genres)
  * [Modded](/wiki/Category:Mods_by_game)
  * [Cheats](/wiki/Category:Cheat_Codes)

###  Modding

  * [Programs](/wiki/Category:Modding_Tools)
  * [Tutorials](/wiki/Category:Tutorials)
  * [Community portal](/wiki/ModdingWiki:Community_portal "About the project, what you can do, where to find things")

###  Programming

  * [File formats](/wiki/Category:File_Formats)

###  Help needed

  * [Known info to be added](/wiki/Category:Stubs)
  * [More info needed](/wiki/Category:Need_more_info)
  * [Unmoddable games](/wiki/Category:Unmoddable)
  * [Images needed](/wiki/Category:Need_pictures)
  * [Pages with TODOs](/wiki/Category:TODO)

###  Wiki

  * [Editing guidelines](/wiki/ModdingWiki:Contributing)
  * [Recent changes](/wiki/Special:RecentChanges "A list of recent changes in the wiki \[r\]")
  * [Random page](/wiki/Special:Random "Load a random page \[x\]")
  * [Help](https://www.mediawiki.org/wiki/Special:MyLanguage/Help:Contents "The place to find out")

###  Tools

  * [What links here](/wiki/Special:WhatLinksHere/ART_Format_\(Build\) "A list of all wiki pages that link here \[j\]")
  * [Related changes](/wiki/Special:RecentChangesLinked/ART_Format_\(Build\) "Recent changes in pages linked from this page \[k\]")
  * [Special pages](/wiki/Special:SpecialPages "A list of all special pages \[q\]")
  * [Printable version](javascript:print\(\); "Printable version of this page \[p\]")
  * [Permanent link](/w/index.php?title=ART_Format_\(Build\)&oldid=8779 "Permanent link to this revision of this page")
  * [Page information](/w/index.php?title=ART_Format_\(Build\)&action=info "More information about this page")
  * [Browse properties](/wiki/Special:Browse/:ART-5FFormat-5F\(Build\))

  * This page was last modified on 4 August 2019, at 13:45.

  * [Privacy policy](/wiki/ModdingWiki:Privacy_policy)
  * [About ModdingWiki](/wiki/ModdingWiki:About)
  * [Disclaimers](/wiki/ModdingWiki:General_disclaimer)

  * [![Powered by MediaWiki](/w/resources/assets/poweredby_mediawiki_88x31.png)](https://www.mediawiki.org/)[![Powered by Semantic MediaWiki](/w/extensions/SemanticMediaWiki/res/smw/logo_footer.png)](https://www.semantic-mediawiki.org/wiki/Semantic_MediaWiki)


#  PNG

From Just Solve the File Format Problem

Jump to: navigation, search

[File Format](/wiki/File_Formats "File Formats")  
---  
Name | PNG  
Ontology  | 

  * [Electronic File Formats](/wiki/Electronic_File_Formats "Electronic File Formats")
    * [Graphics](/wiki/Graphics "Graphics")
      * **PNG**

  
[Extension(s)](/wiki/Filename_extension "Filename extension") | `.png`  
[MIME Type(s)](/wiki/MIME_types "MIME types") | [image/png](http://www.iana.org/assignments/media-types/image/png)  
LoCFDD | [fdd000153](https://www.loc.gov/preservation/digital/formats/fdd/fdd000153.shtml)  
[PRONOM](/wiki/PRONOM "PRONOM") | [fmt/13](https://www.nationalarchives.gov.uk/PRONOM/fmt/13), [fmt/12](https://www.nationalarchives.gov.uk/PRONOM/fmt/12), [fmt/11](https://www.nationalarchives.gov.uk/PRONOM/fmt/11)  
[Wikidata ID](/wiki/Wikidata_identifier "Wikidata identifier") | [Q178051](https://www.wikidata.org/wiki/Q178051)  
[Kaitai Struct Spec](/wiki/Kaitai_Struct "Kaitai Struct") | [png.ksy](http://formats.kaitai.io/png/index.html)  
Released | 1996  
  
**Portable Network Graphics** (**PNG**) was devised starting in a discussion
on newsgroup _comp.graphics_ in 1995, with the first version of its
specification released in 1996. The motivation for its creation was to create
a free and unencumbered image format in the wake of the patent issue with
[GIF](/wiki/GIF "GIF").

PNG has become a very popular graphic format, but widespread adoption on the
Web was slow due to the fact that the first specification came out over a year
after the Web had begun to be popular with the general public, meaning that
there were many sites and browsers out there not using and supporting the new
format; subsequently, browsers began to support it, but often had rendering
problems which persisted even in fairly late versions years later; this caused
webmasters to be slow to switch from GIF to PNG, though many eventually did
so. Since the [LZW](/wiki/LZW "LZW") patent that affected GIF is expired now,
the "free format" motivation for the switch no longer applies.

Unlike GIF, PNG officially supports only still graphics, not animation.
However [APNG](/wiki/APNG "APNG"), an unofficial extension of the PNG image
format that retains the .png file extention does support animation. Another
related format, [MNG](/wiki/MNG "MNG"), officially does support animation.

## Contents

  * 1 Format details
  * 2 Identification
  * 3 Extensions
    * 3.1 Extensions by chunk type
    * 3.2 Other extensions
  * 4 Related Formats
  * 5 Specifications
  * 6 Metaformat files
  * 7 Software
  * 8 Sample files
  * 9 Links

  
---  
  
##  Format details

A PNG file consists of an 8-byte signature, followed by a sequence of
_chunks_. Each chunk has an 8-byte header containing a 4-byte chunk length,
and a 4-byte [chunk type code](/wiki/FourCC "FourCC"). Each chunk also has a
4-byte trailer containing a checksum.

##  Identification

A file begins with an 8-byte signature: `89 50 4E 47 0D 0A 1A 0A`.

A standard PNG file also has ASCII "`IHDR`" at offset 12. You can check for
this to distinguish it from [CgBI](/wiki/CgBI "CgBI").

##  Extensions

###  Extensions by chunk type

Chunk type  |  References and remarks   
---|---  
`oFFs`, `pCAL`, `sCAL`, `gIFg`, `gIFx`, `gIFt`, `fRAc` |  Refer to [Extensions to the PNG 1.2 Specification, v1.2.0](http://pmt.sourceforge.net/specs/pngext-1.2.0-pdg-h20.html).   
`sTER` |  Refer to [Extensions to the PNG 1.2 Specification, v1.3.0](ftp://ftp.simplesystems.org/pub/libpng/png/documents/pngext-1.3.0-pdg.html).   
`dSIG` |  Refer to [Extensions to the PNG 1.2 Specification, v1.4.0](ftp://ftp.simplesystems.org/pub/libpng/png/documents/pngext-1.4.0-pdg.html), and the [PNG dSIG website](http://png-dsig.sourceforge.net/).   
`acTL`, `fcTL`, `fdAT` |  Used in [APNG](/wiki/APNG "APNG") files.   
`vpAg`, `caNv`, `orNT` |  Used by [ImageMagick](/wiki/ImageMagick "ImageMagick").   
`CgBI` |  Refer to [CgBI](/wiki/CgBI "CgBI").   
`eXIf`, `exIf` |  [Exif](/wiki/Exif "Exif") metadata. Refer to [PNG Proposed eXIf chunk](http://ftp-osl.osuosl.org/pub/libpng/documents/proposals/eXIf/png-proposed-eXIf-chunk-2017-06-15.html) [approved 2017-07].   
`iDOT` |  Used by Apple products. Some info at [[1]](https://www.hackerfactor.com/blog/index.php?/archives/895-Connecting-the-iDOTs.html).   
`cpIp`, `cmOD` |  Used in [PNG Plus](/wiki/PNG_Plus "PNG Plus").   
`prVW`, `mkBF`, `mkBS`, `mkBT`, `mkTS` |  Used in [Fireworks PNG](/wiki/Fireworks_PNG "Fireworks PNG").   
  
###  Other extensions

  * [XMP](/wiki/XMP "XMP") metadata can be stored in an `iTXt` chunk with keyword "XML:com.adobe.xmp". Refer to the [XMP Specification](/wiki/XMP#Specifications "XMP"), Part 3. 

##  Related Formats

  * [APNG](/wiki/APNG "APNG")
  * [MNG](/wiki/MNG "MNG")
  * [JNG](/wiki/JNG "JNG")
  * [CgBI](/wiki/CgBI "CgBI")
  * [Portable Bitmap Format](/wiki/Portable_Bitmap_Format "Portable Bitmap Format")
  * [zlib](/wiki/Zlib "Zlib")-style [DEFLATE](/wiki/DEFLATE "DEFLATE") compression: Used to compress image and other data. 
  * [CRC-32](/wiki/CRC-32 "CRC-32"): Used to calculate a checksum of each chunk. 
  * [ICC profile](/wiki/ICC_profile "ICC profile"): The format used by iCCP chunks. 

##  Specifications

  * [W3C PNG specification](http://www.w3.org/TR/PNG/) (latest version) 
    * Specific versions: [1996-10-01](http://www.w3.org/TR/REC-png-961001) · [2003-05-20](http://www.w3.org/TR/2003/PR-PNG-20030520/) · [2003-11-10](http://www.w3.org/TR/2003/REC-PNG-20031110/)
  * [RFC 2083](//tools.ietf.org/html/rfc2083): PNG Specification Version 1.0 
  * [ISO/IEC 15948:2004](http://www.iso.org/iso/iso_catalogue/catalogue_tc/catalogue_detail.htm?csnumber=29581) (not free to download) 
  * [PNG Version 3, update as of June 2025](https://www.w3.org/TR/png-3/)

##  Metaformat files

  * [Kaitai Struct Spec](http://formats.kaitai.io/png/index.html)
  * [Synalysis grammar file](https://raw.githubusercontent.com/synalysis/Grammars/master/png.grammar) (for Hexinator / Synalize It!; [more details](/wiki/Synalysis_grammar_file "Synalysis grammar file")) 

##  Software

_Support for PNG is ubiquitous. Software listed here may have been selected
arbitrarily._

  * [libpng](http://www.libpng.org/pub/png/libpng.html) and [zlib](http://www.zlib.net/)
  * [pngcheck: official PNG tester and debugger](http://www.libpng.org/pub/png/apps/pngcheck.html)
  * [TweakPNG: low-level utility for examining and modifying PNG image files](http://entropymine.com/jason/tweakpng/)
  * [LodePNG](http://lodev.org/lodepng/)
  * [libspng](https://libspng.org/)
  * [Bad Peggy: scans images for problems](http://coptr.digipres.org/Bad_Peggy)
  * [PNGtools: low-level manipulation of PNG structure](https://www.madebymikal.com/pngtools-0-4/)
  * [PNGThermal: indicates compression cost per pixel](https://encode.su/threads/1725-pngthermal-pseudo-thermal-view-of-PNG-compression-efficiency)
  * [Konvertor](/wiki/Konvertor "Konvertor")

##  Sample files

  * [PNG Images](http://www.libpng.org/pub/png/png-sitemap.html#images)
  * [pngimg.com](http://pngimg.com/)
  * [dexvert samples — image/png](http://sembiance.com/fileFormatSamples/image/png)

##  Links

  * [PNG website](http://www.libpng.org/pub/png/png.html)
  * [Portable Network Graphics (Wikipedia)](http://en.wikipedia.org/wiki/Portable_Network_Graphics "wikipedia:Portable Network Graphics")
  * [PNG File Format Summary](https://www.fileformat.info/format/png/egff.htm), from the [Encyclopedia of Graphics File Formats](/wiki/Encyclopedia_of_Graphics_File_Formats "Encyclopedia of Graphics File Formats")
  * [Forensics Wiki article](https://forensics.wiki/portable_network_graphics_%28png%29/)
  * [PNG format mini-poster](http://imgur.com/a/MtQZv#7)
  * [How to repair a PNG that has suffered DOS->Unix character conversion](https://mattscodecave.com/posts/plaidctf-2015---corrupt-png.html)
  * [Getting the Most Out of PNG (Jeff Atwood)](http://blog.codinghorror.com/getting-the-most-out-of-png/)
  * [How PNG Works](https://medium.com/@duhroach/how-png-works-f1174e3cc7b7#.n3l0wota6)
  * [Hello, PNG!](https://www.da.vidbuchanan.co.uk/blog/hello-png.html)

Retrieved from
"[http://fileformats.archiveteam.org/index.php?title=PNG&oldid=50462](http://fileformats.archiveteam.org/index.php?title=PNG&oldid=50462)"

[Categories](/wiki/Special:Categories "Special:Categories"):

  * [File Formats](/wiki/Category:File_Formats "Category:File Formats")
  * [Electronic File Formats](/wiki/Category:Electronic_File_Formats "Category:Electronic File Formats")
  * [Graphics](/wiki/Category:Graphics "Category:Graphics")
  * [File formats with extension .png](/wiki/Category:File_formats_with_extension_.png "Category:File formats with extension .png")

##### Personal tools

  * [Log in / create account](/index.php?title=Special:UserLogin&returnto=PNG "You are encouraged to log in; however, it is not mandatory \[o\]")

##### Namespaces

  * [Page](/wiki/PNG "View the content page \[c\]")
  * [Discussion](/index.php?title=Talk:PNG&action=edit&redlink=1 "Discussion about the content page \[t\]")

####

##### Variants

##### Views

  * [Read](/wiki/PNG)
  * [View source](/index.php?title=PNG&action=edit "This page is protected.
You can view its source \[e\]")

  * [View history](/index.php?title=PNG&action=history "Past revisions of this page \[h\]")

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

  * [What links here](/wiki/Special:WhatLinksHere/PNG "A list of all wiki pages that link here \[j\]")
  * [Related changes](/wiki/Special:RecentChangesLinked/PNG "Recent changes in pages linked from this page \[k\]")
  * [Special pages](/wiki/Special:SpecialPages "A list of all special pages \[q\]")
  * [Printable version](/index.php?title=PNG&printable=yes)
  * [Permanent link](/index.php?title=PNG&oldid=50462 "Permanent link to this revision of the page")

  * This page was last modified on 25 June 2025, at 19:58.
  * This page has been accessed 123,903 times.
  * Content is available under [Creative Commons 0](http://creativecommons.org/publicdomain/zero/1.0/).

  * [Privacy policy](/wiki/Just_Solve_the_File_Format_Problem:Privacy_policy "Just Solve the File Format Problem:Privacy policy")
  * [About Just Solve the File Format Problem](/wiki/Just_Solve_the_File_Format_Problem:About "Just Solve the File Format Problem:About")
  * [Disclaimers](/wiki/Just_Solve_the_File_Format_Problem:General_disclaimer "Just Solve the File Format Problem:General disclaimer")

  * [![Creative Commons 0](http://www.mediawiki.org/w/skins/common/images/cc-0.png)](http://creativecommons.org/publicdomain/zero/1.0/)
  * [![Powered by MediaWiki](/skins/common/images/poweredby_mediawiki_88x31.png)](//www.mediawiki.org/)


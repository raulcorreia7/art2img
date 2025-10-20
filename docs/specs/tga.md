#  TGA

From Just Solve the File Format Problem

Jump to: navigation, search

[File Format](/wiki/File_Formats "File Formats")  
---  
Name | TGA  
Ontology  | 

  * [Electronic File Formats](/wiki/Electronic_File_Formats "Electronic File Formats")
    * [Graphics](/wiki/Graphics "Graphics")
      * **TGA**

  
[Extension(s)](/wiki/Filename_extension "Filename extension") | `.tga`, `.icb`, `.vda`, `.vst`  
LoCFDD | [fdd000179](https://www.loc.gov/preservation/digital/formats/fdd/fdd000179.shtml), [fdd000180](https://www.loc.gov/preservation/digital/formats/fdd/fdd000180.shtml)  
[PRONOM](/wiki/PRONOM "PRONOM") | [x-fmt/367](https://www.nationalarchives.gov.uk/PRONOM/x-fmt/367), [fmt/402](https://www.nationalarchives.gov.uk/PRONOM/fmt/402)  
[Wikidata ID](/wiki/Wikidata_identifier "Wikidata identifier") | [Q1063976](https://www.wikidata.org/wiki/Q1063976)  
[Kaitai Struct Spec](/wiki/Kaitai_Struct "Kaitai Struct") | [tga.ksy](http://formats.kaitai.io/tga/index.html)  
Released | 1984  
  
**TGA** (**Targa**) is a raster image file format developed by Truevision,
Inc. (then named EPICenter) in 1984. Designed for use with MS-DOS color
applications, TGA is the native format of Truevision's TARGA (Truevision
Advanced Raster Graphics Adapter) boards, which were some of the first graphic
cards for IBM-compatible PCs to support 24-bit RGB color encoding (sometimes
termed _truecolor_).

Most TGA files are quite simple, but the format has the potential to be fairly
complex.

TGA images are either uncompressed, or compressed with [run-length
encoding](/wiki/Run-length_encoding "Run-length encoding").

## Contents

  * 1 Format details
  * 2 Variant formats
    * 2.1 ICB
    * 2.2 VDA
    * 2.3 VST
    * 2.4 PIX, BPX
    * 2.5 IVB
  * 3 Identification
  * 4 Versions
  * 5 Specifications
  * 6 Software
  * 7 Sample files
  * 8 Links

  
---  
  
##  Format details

Files begin with an 18-byte fixed header, sometimes followed by other
segments, followed by the pixel data. After the pixel data, there may be
additional data elements referred to by an optional 26-byte file footer.

##  Variant formats

Some Truevision products used their own variant or subset of TGA: **ICB** ,
**VDA** , or **VST**. Those and other TGA variants are listed here.

###  ICB

  * File extension: **.icb**
  * Full name: **Image Capture Board**

###  VDA

  * File extension: **.vda**
  * Full name: **Video Display Adapter**

###  VST

  * File extension: **.vst**
  * Full name: **TrueVista** or **Truevision Vista**

[XnView](/wiki/XnView "XnView") can read and write VST format. The format
XnView supports seems to have more differences from TGA than are mentioned in
the TGA 2.0 specification. There is an extra 18-byte header or ID field after
the main header, with the signature "`IGCH`" at file offset 20.

###  PIX, BPX

  * File extensions: **.pix** , **.bpx**

Refer to [Lumena PIX/BPX](/wiki/Lumena_PIX/BPX "Lumena PIX/BPX").

###  IVB

  * File extension: **.ivb**

No details known. IVB is a file extension or variant format that
[XnView](/wiki/XnView "XnView") claims to support.

##  Identification

TGA files have no signature at the beginning of the file. They can be
identified fairly reliably by testing whether the first 18 bytes have sensible
values for TGA format, but that is nontrivial.

Some, but not all, TGA files have a signature at the _end_ of the file. In
that case, the last 18 bytes of the file are the ASCII characters
"`TRUEVISION-XFILE.`", followed by a NUL byte (`0x00`). An example of a file
ending in `TRUEVISION-XFILE.[0x00]` is seen at [dexvert samples —
image/tga](http://sembiance.com/fileFormatSamples/image/tga) (linked below in
§Sample files) > <https://sembiance.com/fileFormatSamples/image/tga/test.tga>.

##  Versions

The first TGA format is now known as _Original TGA Format_ , or (informally)
_TGA Version 1_. It is characterized by the lack of a _New TGA Format_
signature.

_New TGA Format_ , or _TGA Version 2.0_ , was released in 1989. It is
characterized by a footer with a "TRUEVISION-XFILE" signature. It adds an
optional "Extension Area" segment, with many standard metadata fields.

It also adds an optional "Developer Area" segment, which supports arbitrary
custom data. A custom data item is tagged with a 16-bit integer identifier,
similar to a [TIFF](/wiki/TIFF "TIFF") tag. There does not appear to be any
published list of TGA tags, though tag 20 seems to be used for [Photoshop
Image Resources](/wiki/Photoshop_Image_Resources "Photoshop Image Resources").

##  Specifications

  * Truevision TGA File Format Specification, Version 2.0: [PostScript](http://googlesites.inequation.org/tgautilities) · [PDF](http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf) · [HTML](http://www.ludorg.net/amnesia/TGA_File_Format_Spec.html)
  * [Information extracted by Martin Reddy from Appendix C of the Truevision Technical Guide](http://www.martinreddy.net/gfx/2d/TGA.txt)
  * [Another copy of the file](http://www.textfiles.com/programming/FORMATS/targafor.pro)
  * [Picture format docs (of a number of formats including this one)](http://www.textfiles.com/programming/FORMATS/pix_fmt.txt)

##  Software

TGA is widely supported. Software listed here has been semi-arbitrarily
selected.

  * [Netpbm](/wiki/Netpbm "Netpbm"): pamtotga, tgatoppm 
  * [ImageMagick](/wiki/ImageMagick "ImageMagick")
  * [FFmpeg](/wiki/FFmpeg "FFmpeg")
  * [XnView](/wiki/XnView "XnView")
  * [Libtga](http://tgalib.sourceforge.net/)
  * [Deark](https://entropymine.com/deark/)
  * [abydos](http://snisurset.net/code/abydos/)
  * [GIMP](/wiki/GIMP "GIMP")

##  Sample files

  * <http://www.fileformat.info/format/tga/sample/index.htm>
  * <https://samples.libav.org/image-samples/TGA/>
  * <http://links.uwaterloo.ca/Repository/TGA/>
  * <http://downloads.oldschoolbg.com/cstrike/gfx/env/>
  * <https://github.com/timfel/tombexcavator/tree/master/data/TGA>
  * [dexvert samples — image/tga](http://sembiance.com/fileFormatSamples/image/tga)

##  Links

  * [Truevision TGA: Wikipedia](http://en.wikipedia.org/wiki/Truevision_TGA "wikipedia:Truevision TGA")
  * [TGA File Format Summary](https://www.fileformat.info/format/tga/egff.htm), from the [Encyclopedia of Graphics File Formats](/wiki/Encyclopedia_of_Graphics_File_Formats "Encyclopedia of Graphics File Formats")
  * [Creating TGA Image files](http://www.paulbourke.net/dataformats/tga/) By Paul Bourke, 1996. 
  * [TGA format chart](https://twitter.com/angealbertini/status/535565222652948480/photo/1)

Retrieved from
"[http://fileformats.archiveteam.org/index.php?title=TGA&oldid=47317](http://fileformats.archiveteam.org/index.php?title=TGA&oldid=47317)"

[Categories](/wiki/Special:Categories "Special:Categories"):

  * [File Formats](/wiki/Category:File_Formats "Category:File Formats")
  * [Electronic File Formats](/wiki/Category:Electronic_File_Formats "Category:Electronic File Formats")
  * [Graphics](/wiki/Category:Graphics "Category:Graphics")
  * [File formats with extension .tga](/wiki/Category:File_formats_with_extension_.tga "Category:File formats with extension .tga")
  * [File formats with extension .icb](/wiki/Category:File_formats_with_extension_.icb "Category:File formats with extension .icb")
  * [File formats with extension .vda](/wiki/Category:File_formats_with_extension_.vda "Category:File formats with extension .vda")
  * [File formats with extension .vst](/wiki/Category:File_formats_with_extension_.vst "Category:File formats with extension .vst")

Hidden category:

  * [FormatInfo without mimetypes](/wiki/Category:FormatInfo_without_mimetypes "Category:FormatInfo without mimetypes")

##### Personal tools

  * [Log in / create account](/index.php?title=Special:UserLogin&returnto=TGA "You are encouraged to log in; however, it is not mandatory \[o\]")

##### Namespaces

  * [Page](/wiki/TGA "View the content page \[c\]")
  * [Discussion](/wiki/Talk:TGA "Discussion about the content page \[t\]")

####

##### Variants

##### Views

  * [Read](/wiki/TGA)
  * [View source](/index.php?title=TGA&action=edit "This page is protected.
You can view its source \[e\]")

  * [View history](/index.php?title=TGA&action=history "Past revisions of this page \[h\]")

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

  * [What links here](/wiki/Special:WhatLinksHere/TGA "A list of all wiki pages that link here \[j\]")
  * [Related changes](/wiki/Special:RecentChangesLinked/TGA "Recent changes in pages linked from this page \[k\]")
  * [Special pages](/wiki/Special:SpecialPages "A list of all special pages \[q\]")
  * [Printable version](/index.php?title=TGA&printable=yes)
  * [Permanent link](/index.php?title=TGA&oldid=47317 "Permanent link to this revision of the page")

  * This page was last modified on 28 December 2023, at 04:40.
  * This page has been accessed 51,298 times.
  * Content is available under [Creative Commons 0](http://creativecommons.org/publicdomain/zero/1.0/).

  * [Privacy policy](/wiki/Just_Solve_the_File_Format_Problem:Privacy_policy "Just Solve the File Format Problem:Privacy policy")
  * [About Just Solve the File Format Problem](/wiki/Just_Solve_the_File_Format_Problem:About "Just Solve the File Format Problem:About")
  * [Disclaimers](/wiki/Just_Solve_the_File_Format_Problem:General_disclaimer "Just Solve the File Format Problem:General disclaimer")

  * [![Creative Commons 0](http://www.mediawiki.org/w/skins/common/images/cc-0.png)](http://creativecommons.org/publicdomain/zero/1.0/)
  * [![Powered by MediaWiki](/skins/common/images/poweredby_mediawiki_88x31.png)](//www.mediawiki.org/)


#  BMP

From Just Solve the File Format Problem

Jump to: navigation, search

[File Format](/wiki/File_Formats "File Formats")  
---  
Name | BMP  
Ontology  | 

  * [Electronic File Formats](/wiki/Electronic_File_Formats "Electronic File Formats")
    * [Graphics](/wiki/Graphics "Graphics")
      * **BMP**

  
[Extension(s)](/wiki/Filename_extension "Filename extension") | `.bmp`, `.rle`, `.dib`, others  
[MIME Type(s)](/wiki/MIME_types "MIME types") | [image/bmp](http://www.iana.org/assignments/media-types/image/bmp)  
LoCFDD | [fdd000189](https://www.loc.gov/preservation/digital/formats/fdd/fdd000189.shtml)  
[PRONOM](/wiki/PRONOM "PRONOM") | [fmt/116](https://www.nationalarchives.gov.uk/PRONOM/fmt/116), others  
[Wikidata ID](/wiki/Wikidata_identifier "Wikidata identifier") | [Q192869](https://www.wikidata.org/wiki/Q192869)  
[Kaitai Struct Spec](/wiki/Kaitai_Struct "Kaitai Struct") | [bmp.ksy](http://formats.kaitai.io/bmp/index.html)  
Released | 1987  
  
**BMP** is a family of raster image file formats primarily used on Microsoft
Windows and OS/2 operating systems. The format is sometimes known as **Device-
Independent Bitmap** (**DIB**), since, when loaded into memory using Windows
software, the image is held as a DIB structure.

Though seemingly a simple format, it is complicated by its many different
versions, lack of an official specification, lack of any version control
process, and ambiguities and contradictions in the documentation.

## Contents

  * 1 Discussion
  * 2 Format details
    * 2.1 Compression
    * 2.2 File structure
  * 3 Identifiers
  * 4 Identification
  * 5 Well-known versions
    * 5.1 Windows BMP v2
    * 5.2 Windows BMP v3
    * 5.3 Windows BMP v4
    * 5.4 Windows BMP v5
  * 6 Other versions
    * 6.1 Windows BMP v1
    * 6.2 OS/2 BMP 1.0
    * 6.3 OS/2 BMP 2.0
    * 6.4 BITMAPV2INFOHEADER
    * 6.5 BITMAPV3INFOHEADER
    * 6.6 Packed DIB file
    * 6.7 OS/2 Bitmap Array
  * 7 Symbol definitions
  * 8 Specifications
  * 9 Metaformat files
  * 10 Software
  * 11 Sample files
    * 11.1 Windows BMP v2
    * 11.2 Windows BMP v3
    * 11.3 OS/2 BMP 2.0
    * 11.4 Various
  * 12 See also
  * 13 Links

  
---  
  
##  Discussion

The term _DIB_ can mean several different things:

  * A synonym for BMP file format. 
  * An in-memory DIB object, with no file header. The header data and the bitmap data do not have to be stored contiguously. Some Win32 API functions use this format. 
  * A "packed DIB" memory object or file component, with no file header. The header data and the bitmap data are stored contiguously. This is a standard [clipboard format](/wiki/Windows_clipboard "Windows clipboard"), for example. 
  * A "packed DIB" stored in a file by itself. 

A number of Windows-centric formats contain some nonstandard modified or
compressed form of BMP/DIB, intended to be reconstructed as a DIB at runtime.

##  Format details

###  Compression

Images are usually uncompressed, but [RLE](/wiki/Run-length_encoding "Run-
length encoding") compression can be used under some conditions.
[JPEG](/wiki/JPEG "JPEG"), [PNG](/wiki/PNG "PNG"), and [Huffman
1D](/wiki/Modified_Huffman "Modified Huffman") compression are also
theoretically possible, but rarely supported.

###  File structure

A BMP file starts with a 14-byte "BITMAPFILEHEADER" structure.

Immediately after that is another header which we'll refer to as the "info
header", though some versions of it are named "core header" instead. There are
a number of different versions and sub-versions of it. It starts with 4-byte
integer indicating its size, which _mostly_ reveals its version.

The pixel data is pointed to by a field in BITMAPFILEHEADER. There can be
other data segments, e.g. for a color palette, before (and, rarely, after) the
pixel data.

Pixel data is usually stored from bottom up (but is top-down if the header
indicates a negative height). If uncompressed, each row is padded to a
multiple of 4 bytes.

##  Identifiers

No MIME type has been officially registered. Strings found in practice are:
image/bmp; image/x-bmp; image/x-ms-bmp

The usual filename extension is **.bmp**. Extensions **.rle** (for RLE-
compressed images) and **.dib** (which sometimes indicates that the file lacks
a _file header_) are also sometimes seen. Many other extensions have been used
by various applications.

##  Identification

BMP files start with the ASCII signature "`BM`".

The bytes at the beginning of the file match one of these two patterns:

    
    
    42 4d ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 0c 00 
    00 00 ?? ?? ?? ?? 01 00 ?? 00
    

or

    
    
    42 4d ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 ?? 00 
    00 00 ?? ?? ?? ?? ?? ?? ?? ?? 01 00 ?? 00
    

The "reserved" bytes at offset 6 to 9 are usually all zero, but a few OS/2 BMP
files have nonzero "hotspot coordinates" there.

Compare to [VBM (VDC BitMap)](/wiki/VBM_\(VDC_BitMap\) "VBM \(VDC BitMap\)"),
an obscure format with a similar signature.

##  Well-known versions

###  Windows BMP v2

  * Info header size: 12 
  * Info header name: BITMAPCOREHEADER 
  * PRONOM: [fmt/115](https://www.nationalarchives.gov.uk/PRONOM/fmt/115)

See also OS/2 BMP 1.0, below.

###  Windows BMP v3

  * Info header size: 40 
  * Info header name: BITMAPINFOHEADER 
  * PRONOM: [fmt/116](https://www.nationalarchives.gov.uk/PRONOM/fmt/116), [fmt/117](https://www.nationalarchives.gov.uk/PRONOM/fmt/117)

This is by far the most widely used version of BMP. It was introduced with
Windows 3.x. Windows NT extended it to support 16 and 32 bits/pixel.

Windows CE also extended it, for example to allow 2 bits/pixel (see [Pocket PC
Bitmap](/wiki/Pocket_PC_Bitmap "Pocket PC Bitmap")), but its extensions were
not migrated to the BMP v4 and v5 formats.

Microsoft's GDI+ reportedly supports BMP images with 64
bits/pixel[[1]](https://docs.microsoft.com/en-
us/windows/win32/gdiplus/-gdiplus-types-of-bitmaps-about), but no technical
documentation of this extension has been located.

It is apparently possible for OS/2 BMP 2.0 format to masquerade as Windows BMP
v3. The upshot is that if the compression type is 3 and the bit depth is 1, or
the compression type is 4 and the bit depth is 24, then the file should be
treated as OS/2 BMP 2.0.

###  Windows BMP v4

  * Info header size: 108 
  * Info header name: BITMAPV4HEADER 
  * PRONOM: [fmt/118](https://www.nationalarchives.gov.uk/PRONOM/fmt/118)

Introduced with Windows 95. Adds support for transparency and colorimetry.

###  Windows BMP v5

  * Info header size: 124 
  * Info header name: BITMAPV5HEADER 
  * PRONOM: [fmt/119](https://www.nationalarchives.gov.uk/PRONOM/fmt/119)

Introduced with Windows 98. Adds support for [ICC profiles](/wiki/ICC_profile
"ICC profile").

##  Other versions

###  Windows BMP v1

  * PRONOM: [fmt/114](https://www.nationalarchives.gov.uk/PRONOM/fmt/114)

Also called DDB, this is the bitmap file format used by Windows 1.0. It's not
really a BMP format. Refer to [Windows DDB](/wiki/Windows_DDB "Windows DDB").

###  OS/2 BMP 1.0

    _See also the[OS/2 BMP disambiguation page](/wiki/OS/2_BMP "OS/2 BMP")._

  * Info header size: 12 
  * Info header name: BITMAPCOREHEADER or OS21XBITMAPHEADER 
  * PRONOM: [x-fmt/25](https://www.nationalarchives.gov.uk/PRONOM/x-fmt/25) (deprecated), [fmt/115](https://www.nationalarchives.gov.uk/PRONOM/fmt/115)

For practical purposes, OS/2 BMP 1.0 is identical to Windows BMP v2. But there
can be at least one small difference: In OS/2 formats, the "Size" field at
offset 2 (variously named "cbSize", "FileSize", or "bfSize") is sometimes set
to the size of the headers, instead of the size of the file. For v1, that
means it will be 26 (14+12). For v2, it can range from 30 to 78.

###  OS/2 BMP 2.0

  * Info header size: 16–64 (16, 24, 40, 48, and 64 may be most common) 
  * Info header name: BITMAPCOREHEADER2 or OS22XBITMAPHEADER 
  * PRONOM: [x-fmt/270](https://www.nationalarchives.gov.uk/PRONOM/x-fmt/270)

OS/2 BMP 2.0 defines several file subtypes; here we are describing only the
"Bitmap" subtype (files with a signature of "BM"). For other subtypes, see
[OS/2 bitmap family](/wiki/OS/2_bitmap_family "OS/2 bitmap family").

The header size can be reduced from its full size of 64 bytes. Omitted fields
are assumed to have a value of zero.

The fields in the first 40 bytes of the header are (nearly) identical to those
in Windows BMP v3, v4, and v5. The remaining fields are different.

OS/2 BMP 2.0 supports compression types "Huffman 1D" and "RLE24", unlike any
other version of BMP.

###  BITMAPV2INFOHEADER

  * Info header size: 52 
  * Info header name: BITMAPV2INFOHEADER 

Uncertain; possibly an abbreviated V4/V5 header.

###  BITMAPV3INFOHEADER

  * Info header size: 56 
  * Info header name: BITMAPV3INFOHEADER 

Uncertain; possibly an abbreviated V4/V5 header.

###  Packed DIB file

Same as the common BMP formats, but omits the 14-byte _file header_.

###  OS/2 Bitmap Array

Sometimes, an [OS/2 Bitmap Array](/wiki/OS/2_Bitmap_Array "OS/2 Bitmap Array")
file containing one or more bitmaps is considered to be a kind of BMP file.
Such a file begins with an extra 14-byte header, with signature "`BA`". (But
note that deleting this header is not quite enough to get a valid BMP file.)

##  Symbol definitions

Here are the definitions, from the Windows SDKs, of some of the symbols used
in the BMP documentation.

All integers use little-endian [byte order](/wiki/Endianness "Endianness").

Symbol  |  Definition   
---|---  
WORD  |  unsigned 16-bit integer   
DWORD  |  unsigned 32-bit integer   
LONG  |  signed 32-bit integer   
BI_RGB  |  0   
BI_RLE8  |  1   
BI_RLE4  |  2   
BI_BITFIELDS  |  3   
(Huffman 1D)  |  3   
BI_JPEG  |  4   
(24-bit RLE)  |  4   
BI_PNG  |  5   
BI_ALPHABITFIELDS  |  6   
BI_SRCPREROTATE  |  0x8000 (?)   
LCS_CALIBRATED_RGB  |  0   
LCS_sRGB  |  `'sRGB'` = 0x73524742   
LCS_WINDOWS_COLOR_SPACE  |  `'Win '` = 0x57696e20   
PROFILE_LINKED  |  `'LINK'` = 0x4c494e4b   
PROFILE_EMBEDDED  |  `'MBED'` = 0x4d424544   
LCS_GM_BUSINESS  |  1   
LCS_GM_GRAPHICS  |  2   
LCS_GM_IMAGES  |  4   
LCS_GM_ABS_COLORIMETRIC  |  8   
  
##  Specifications

  * Most of the format (but not the BITMAPFILEHEADER) is defined in the [Windows Metafile Specification](https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/4813e7fd-52d0-4f42-965f-228c8b7488d2) (as "DeviceIndependentBitmap") 
  * Defined in the [RIFF specification](https://www.aelius.com/njh/wavemetatools/doc/riffmci.pdf) (as Device Independent Bitmap File Format) 
  * [BITMAPFILEHEADER](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader)
  * [BITMAPCOREHEADER](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapcoreheader)
  * [BITMAPINFOHEADER](https://docs.microsoft.com/en-us/previous-versions/dd183376\(v=vs.85\))
  * [BITMAPINFOHEADER (Windows CE 5.0)](https://docs.microsoft.com/en-us/previous-versions/windows/embedded/aa452885\(v=msdn.10\))
  * [BITMAPV4HEADER](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header)
  * [BITMAPV5HEADER](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header)
  * [Another site with format descriptions](http://www.digicamsoft.com/bmp/bmp.html)
  * [OS/2 Presentation Manager Programming Reference Guide, Vol III](http://www.fileformat.info/format/os2bmp/spec/902d5c253f2a43ada39c2b81034f27fd/view.htm)
  * [Specification of BMP headers](http://whatis.rest7.com/how-to-open-bmp-file)
  * [BMP format information by T. Schultz](http://zig.tgschultz.com/bmp_file_format.txt)

##  Metaformat files

  * [Synalysis grammar file](https://raw.githubusercontent.com/synalysis/Grammars/master/bitmap.grammar) (for Hexinator / Synalize It!; [more details](/wiki/Synalysis_grammar_file "Synalysis grammar file")) 

##  Software

BMP is widely supported by graphics software, including web browsers. Software
listed here has been arbitrarily selected.

  * [Netpbm](/wiki/Netpbm "Netpbm"): bmptopnm, ppmtobmp 
  * [ImageMagick](/wiki/ImageMagick "ImageMagick")
  * [XnView](/wiki/XnView "XnView")
  * [Tom's Viewer](/wiki/Tom%27s_Viewer "Tom's Viewer")
  * [Reggae](https://library.morph.zone/Reggae:_MorphOS_multimedia_framework)

##  Sample files

###  Windows BMP v2

This format is fairly common, but examples of it can be hard to spot amidst
all the BMPv3 files.

  * [money-2-(os2).bmp](https://samples.libav.org/image-samples/money-2-%28os2%29.bmp), [money-16-(os2).bmp](https://samples.libav.org/image-samples/money-16-%28os2%29.bmp), [money-256-(os2).bmp](https://samples.libav.org/image-samples/money-256-%28os2%29.bmp), [money-24bit-os2.bmp](https://samples.libav.org/image-samples/money-24bit-os2.bmp)
  * <http://cd.textfiles.com/hobbesos29709/disk2/MULTIMED/BMP/> → BMPS*.ZIP, OS2ORBIT.ZIP, REALMERL.BMP 

###  Windows BMP v3

  * <http://cd.textfiles.com/mmplatinum/IMAGES/BMP/>
  * <http://cd.textfiles.com/monstmedia/WIN/ICONS/>
  * <http://cd.textfiles.com/maxx/tothemaxww/BMPMISC/>

###  OS/2 BMP 2.0

  * [teamlog2.zip](http://kimludvigsen.dk/os2/os2/graphx/teamlog2.zip) → TEAMcol.bmp, teamcol2.bmp 
  * [test4os2v2.bmp](https://samples.libav.org/image-samples/bmp-files/test4os2v2.bmp)
  * [CARDBMPS.ZIP](http://cd.textfiles.com/hobbesos29709/disk2/MULTIMED/BMP/CARDBMPS.ZIP) \- Most of the files are OS/2 BMP 2.0 
  * [MAKMAN10.ZIP](http://cd.textfiles.com/hobbesos29709/disk1/GAMES/MAKMAN10.ZIP) \- Some of the files are files OS/2 BMP 2.0 
  * [JUR_OS2.BMP](http://cd.textfiles.com/monstmedia/IMAGES/JUR_OS2.BMP)
  * <http://cd.textfiles.com/pier/pier09/cdrom/036/> → os2bmp*.zip 

###  Various

  * [Example BMP images (all Windows v3 except as indicated)](http://wvnvms.wvnet.edu/vmswww/bmp.html)
  * [OS/2 BBS Files Archive](https://archive.org/details/OS2BBS) → 13-Bitmap.zip - Lots of OS/2 BMP 1.0 & 2.0 files 
  * [BMP Suite Image List](https://entropymine.com/jason/bmpsuite/bmpsuite/html/bmpsuite.html)
  * [dexvert samples — image/bmp](http://sembiance.com/fileFormatSamples/image/bmp)

##  See also

  * [CUR](/wiki/CUR "CUR")
  * [ICO](/wiki/ICO "ICO")
  * [KQP](/wiki/KQP "KQP")
  * [OS/2 bitmap family](/wiki/OS/2_bitmap_family "OS/2 bitmap family")
  * [OS/2 Bitmap Array](/wiki/OS/2_Bitmap_Array "OS/2 Bitmap Array")
  * [packPNM](/wiki/PackPNM "PackPNM")
  * [Pegasus PIC](/wiki/Pegasus_PIC "Pegasus PIC")
  * [Pocket PC Bitmap](/wiki/Pocket_PC_Bitmap "Pocket PC Bitmap")
  * [Poser Bump Map](/wiki/Poser_Bump_Map "Poser Bump Map")
  * [Segmented Hypergraphics](/wiki/Segmented_Hypergraphics "Segmented Hypergraphics")
  * [Windows DDB](/wiki/Windows_DDB "Windows DDB")
  * [Winzle Puzzle](/wiki/Winzle_Puzzle "Winzle Puzzle")
  * BMP is sometimes used as a base format for [steganography](/wiki/Steganography "Steganography") and similar things. For examples, see: 
    * [Data Hiding/Embedding#BMP](/wiki/Data_Hiding/Embedding#BMP "Data Hiding/Embedding")
    * [Encryption#Obfuscation](/wiki/Encryption#Obfuscation "Encryption")
    * [Encryption#Steganography](/wiki/Encryption#Steganography "Encryption")

##  Links

  * [Wikipedia: BMP file format](http://en.wikipedia.org/wiki/BMP_file_format "wikipedia:BMP file format")
  * [Microsoft Windows Bitmap File Format Summary](https://www.fileformat.info/format/bmp/egff.htm), from the [Encyclopedia of Graphics File Formats](/wiki/Encyclopedia_of_Graphics_File_Formats "Encyclopedia of Graphics File Formats")
  * [OS/2 Bitmap File Format Summary](https://www.fileformat.info/format/os2bmp/egff.htm), from the [Encyclopedia of Graphics File Formats](/wiki/Encyclopedia_of_Graphics_File_Formats "Encyclopedia of Graphics File Formats")
  * [Bad Peggy: scans images for problems](http://coptr.digipres.org/Bad_Peggy)
  * [ForensicsWiki entry](https://web.archive.org/web/20190919034602/http://forensicswiki.org/wiki/BMP) (not much useful info except for a link to an iOS tool for reading metadata) 
  * [Graphic documenting format](https://github.com/corkami/pics/blob/master/binary/BMP.png)

Retrieved from
"[http://fileformats.archiveteam.org/index.php?title=BMP&oldid=47266](http://fileformats.archiveteam.org/index.php?title=BMP&oldid=47266)"

[Categories](/wiki/Special:Categories "Special:Categories"):

  * [File Formats](/wiki/Category:File_Formats "Category:File Formats")
  * [Electronic File Formats](/wiki/Category:Electronic_File_Formats "Category:Electronic File Formats")
  * [Graphics](/wiki/Category:Graphics "Category:Graphics")
  * [File formats with extension .bmp](/wiki/Category:File_formats_with_extension_.bmp "Category:File formats with extension .bmp")
  * [File formats with extension .rle](/wiki/Category:File_formats_with_extension_.rle "Category:File formats with extension .rle")
  * [File formats with extension .dib](/wiki/Category:File_formats_with_extension_.dib "Category:File formats with extension .dib")
  * [Microsoft](/wiki/Category:Microsoft "Category:Microsoft")
  * [Windows](/wiki/Category:Windows "Category:Windows")
  * [OS/2](/wiki/Category:OS/2 "Category:OS/2")

##### Personal tools

  * [Log in / create account](/index.php?title=Special:UserLogin&returnto=BMP "You are encouraged to log in; however, it is not mandatory \[o\]")

##### Namespaces

  * [Page](/wiki/BMP "View the content page \[c\]")
  * [Discussion](/wiki/Talk:BMP "Discussion about the content page \[t\]")

####

##### Variants

##### Views

  * [Read](/wiki/BMP)
  * [View source](/index.php?title=BMP&action=edit "This page is protected.
You can view its source \[e\]")

  * [View history](/index.php?title=BMP&action=history "Past revisions of this page \[h\]")

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

  * [What links here](/wiki/Special:WhatLinksHere/BMP "A list of all wiki pages that link here \[j\]")
  * [Related changes](/wiki/Special:RecentChangesLinked/BMP "Recent changes in pages linked from this page \[k\]")
  * [Special pages](/wiki/Special:SpecialPages "A list of all special pages \[q\]")
  * [Printable version](/index.php?title=BMP&printable=yes)
  * [Permanent link](/index.php?title=BMP&oldid=47266 "Permanent link to this revision of the page")

  * This page was last modified on 28 December 2023, at 04:20.
  * This page has been accessed 207,275 times.
  * Content is available under [Creative Commons 0](http://creativecommons.org/publicdomain/zero/1.0/).

  * [Privacy policy](/wiki/Just_Solve_the_File_Format_Problem:Privacy_policy "Just Solve the File Format Problem:Privacy policy")
  * [About Just Solve the File Format Problem](/wiki/Just_Solve_the_File_Format_Problem:About "Just Solve the File Format Problem:About")
  * [Disclaimers](/wiki/Just_Solve_the_File_Format_Problem:General_disclaimer "Just Solve the File Format Problem:General disclaimer")

  * [![Creative Commons 0](http://www.mediawiki.org/w/skins/common/images/cc-0.png)](http://creativecommons.org/publicdomain/zero/1.0/)
  * [![Powered by MediaWiki](/skins/common/images/poweredby_mediawiki_88x31.png)](//www.mediawiki.org/)


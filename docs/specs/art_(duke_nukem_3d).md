#  ART (Duke Nukem 3D)

From Just Solve the File Format Problem

Jump to: navigation, search

[File Format](/wiki/File_Formats "File Formats")  
---  
Name | ART (Duke Nukem 3D)  
Ontology  | 

  * [Electronic File Formats](/wiki/Electronic_File_Formats "Electronic File Formats")
    * [Graphics](/wiki/Graphics "Graphics")
      * **ART (Duke Nukem 3D)**

  
[Extension(s)](/wiki/Filename_extension "Filename extension") | `.art`  
  
The **ART** format is a uncompressed archive format used by _Duke Nukem 3D_ (a
[Build Engine](/wiki/Category:Build_Engine_formats "Category:Build Engine
formats") game) to store all of the game's image data (such as textures and
sprites). Each file within the archive is called a **Tile**. Each archive can
contain a maximum of 256 tiles.

Files are typically named `TILES###.ART`, starting with `TILES000.ART`, and
are loaded by the engine in sequence.

Image data within the archive are paletted and uncompressed, and can only be
converted using the engine's `PALETTE.DAT` file (see [Build Engine
Palette](/wiki/DAT_\(Build_Engine_Palette\) "DAT \(Build Engine Palette\)")).
Whilst uncompressed, image data is stored in column-order not row-order. The
255th index in the palette is used for transparency.

## Contents

  * 1 File signature
  * 2 Source
  * 3 Specification
  * 4 See also
  * 5 Sample files

  
---  
  
##  File signature

Whilst the format does not contain a file signature/magic bytes, the version
number (`01 00 00 00`) can be used in its place.

##  Source

The following source can be used to extract `.ART` files using Python. The
script requires two inputs, the ART file and the corresponding `PALETTE.DAT`
file.

    
    
    from PIL import Image as Pillow
    import struct
    import argparse
    import os
    
    header_format = '<llll'
    header_format_size = struct.calcsize(header_format)
    
    tile_dimension_format = "<H"
    tile_dimension_format_size = struct.calcsize(tile_dimension_format)
    
    tile_attribute_format = "<i"
    tile_attribute_format_size = struct.calcsize(tile_attribute_format)
    
    palette_format = "<BBB"
    palette_format_size = struct.calcsize(palette_format)
    
    def palette_to_sequence(fileName, count=256):
    	colors = [None] * count
    	
    	with open(fileName, 'rb') as f:
    		for i in range(0, count):
    			color_tuple = f.read(palette_format_size)
    			r, g, b = struct.unpack(palette_format, color_tuple)
    			
    			# Only 6-bits are used for color information, so each byte will need to be
    			# multiplied by 4
    			colors[i] = [r * 4, g * 4, b * 4]
    		
    	return sum(colors, []) # Flatten the array
    
    
    def unpack(fileName, paletteFileName):
    	BASE_NAME = os.path.splitext(os.path.basename(fileName))[0]
    	PALETTE = palette_to_sequence(paletteFileName)
    
    	try:
    		os.mkdir(BASE_NAME)
    	except:
    		pass
    
    	with open(fileName, 'rb') as f:
    		[version, _, local_tile_start, local_tile_end] = struct.unpack(header_format, f.read(header_format_size))
    		assert version == 1
    		assert local_tile_start > 1
    		assert local_tile_end > local_tile_start
    
    		print('version:', version)
    		print('local_tile_start:', local_tile_start)
    		print('local_tile_end:', local_tile_end)
    
    		tile_count = local_tile_end - local_tile_start + 1
    		assert tile_count <= 256
    
    		print('tile_count:', tile_count)
    
    		tile_widths = []
    		tile_heights = []
    		tile_attributes = []
    
    		# Read tile widths
    		for _ in range(0, tile_count):
    			temp = struct.unpack(tile_dimension_format, f.read(tile_dimension_format_size))
    			tile_widths.append(temp[0])
    
    		# Read tile heights
    		for _ in range(0, tile_count):
    			temp = struct.unpack(tile_dimension_format, f.read(tile_dimension_format_size))
    			tile_heights.append(temp[0])
    
    		# Read tile attributes
    		for _ in range(0, tile_count):
    			temp = struct.unpack(tile_attribute_format, f.read(tile_attribute_format_size))
    			tile_attributes.append(temp[0])
    		
    		# Read tile image data (image data is stored in a column order, not a row order,
    		# so widths and heights are flipped)
    		entry_begin_offset = f.tell()
    		running_length = 0
    
    		for i in range(0, tile_count):
    			w = tile_heights[i]
    			h = tile_widths[i]
    			length = w * h
    			offset = entry_begin_offset + running_length
    			running_length += length
    			
    			tile_number = local_tile_start + i
    			tile_data = f.read(length)
    			file_name = "{}_{}.png".format(tile_number, offset)
    
    			# Transpose data
    			tile_data = list(tile_data)
    			tile_data = [tile_data[i:i + w] for i in range(0, len(tile_data), w)]
    			tile_data = sum(map(list, zip(*tile_data)), [])
    
    			with open(os.path.join(BASE_NAME, file_name), 'wb') as image_output:
    				im = Pillow.new(mode="P", size=(h, w))
    				im.putpalette(PALETTE)
    				im.putdata(tile_data)
    				im.save(image_output, 'png', transparency=255)
    
    
    if __name__ == '__main__':
    	parser = argparse.ArgumentParser(
    					prog = 'ART Extractor',
    					description = 'Unpacks Build Engine .ART files')
    	parser.add_argument('INPUT_FILE')
    	parser.add_argument('PALETTE_FILE')
    	args = parser.parse_args()
    	
    	unpack(args.INPUT_FILE, args.PALETTE_FILE)
    

##  Specification

Ken Silverman, the original author of the Build Engine, provided the following
documentation of the ART format:

    
    
    Documentation on Ken's .ART file format                     by Ken Silverman
    
       I am documenting my ART format to allow you to program your own custom
    art utilites if you so desire.  I am still planning on writing the script
    system.
    
       All art files must have xxxxx###.ART.  When loading an art file you
    should keep trying to open new xxxxx###'s, incrementing the number, until
    an art file is not found.
    
    
    1. long artversion;
    
          The first 4 bytes in the art format are the version number.  The current
       current art version is now 1.  If artversion is not 1 then either it's the
       wrong art version or something is wrong.
    
    2. long numtiles;
    
          Numtiles is not really used anymore.  I wouldn't trust it.  Actually
       when I originally planning art version 1 many months ago, I thought I
       would need this variable, but it turned it is was unnecessary.  To get
       the number of tiles, you should search all art files, and check the
       localtilestart and localtileend values for each file.
    
    3. long localtilestart;
    
          Localtilestart is the tile number of the first tile in this art file.
    
    4. long localtileend;
    
          Localtileend is the tile number of the last tile in this art file.
          Note:  Localtileend CAN be higher than the last used slot in an art
          file.
    
             Example:  If you chose 256 tiles per art file:
          TILES000.ART -> localtilestart = 0,   localtileend = 255
          TILES001.ART -> localtilestart = 256, localtileend = 511
          TILES002.ART -> localtilestart = 512, localtileend = 767
          TILES003.ART -> localtilestart = 768, localtileend = 1023
    
    5. short tilesizx[localtileend-localtilestart+1];
    
          This is an array of shorts of all the x dimensions of the tiles
       in this art file.  If you chose 256 tiles per art file then
       [localtileend-localtilestart+1] should equal 256.
    
    6. short tilesizy[localtileend-localtilestart+1];
    
          This is an array of shorts of all the y dimensions.
    
    7. long picanm[localtileend-localtilestart+1];
    
          This array of longs stores a few attributes for each tile that you
       can set inside EDITART.  You probably won't be touching this array, but
       I'll document it anyway.
    
       Bit:  │31           24│23           16│15            8│7             0│
             ├───────────────┼───────────────┼───────────────┼───────────────┤
             │ | | | | | | | │ | | | | | | | │ | | | | | | | │ | | | | | | | │
             └───────┬───────┼───────────────┼───────────────┼───┬───────────┤
                     │ Anim. │  Signed char  │  Signed char  │   │  Animate  │
                     │ Speed │   Y-center    │   X-center    │   │   number  │
                     └───────┤    offset     │    offset     │   ├───────────┘
                             └───────────────┴───────────────┤   └──────────┐
                                                             │ Animate type:│
                                                             │ 00 - NoAnm   │
                                                             │ 01 - Oscil   │
                                                             │ 10 - AnmFd   │
                                                             │ 11 - AnmBk   │
                                                             └──────────────┘
              You probably recognize these:
           Animate speed -            EDITART key: 'A', + and - to adjust
           Signed char x&y offset -   EDITART key: '`', Arrows to adjust
           Animate number&type -      EDITART key: +/- on keypad
    
    8. After the picanm's, the rest of the file is straight-forward rectangular
          art data.  You must go through the tilesizx and tilesizy arrays to find
          where the artwork is actually stored in this file.
    
          Note:  The tiles are stored in the opposite coordinate system than
             the screen memory is stored.  Example on a 4*4 file:
    
             Offsets:
             ┌───┬───┬───┬───┐
             │ 0 │ 4 │ 8 │12 │
             ├───┼───┼───┼───┤
             │ 1 │ 5 │ 9 │13 │
             ├───┼───┼───┼───┤
             │ 2 │ 6 │10 │14 │
             ├───┼───┼───┼───┤
             │ 3 │ 7 │11 │15 │
             └───┴───┴───┴───┘
    
    ----------------------------------------------------------------------------
       If you wish to display the artwork, you will also need to load your
    palette.  To load the palette, simply read the first 768 bytes of your
    palette.dat and write it directly to the video card - like this:
    
       Example:
          long i, fil;
    
          fil = open("palette.dat",O_BINARY|O_RDWR,S_IREAD);
          read(fil,&palette[0],768);
          close(fil);
    
          outp(0x3c8,0);
          for(i=0;i<768;i++)
             outp(0x3c9,palette[i]);
    

##  See also

  * [_ART Format (Build)_ on the _DOS Game Modding Wiki_](https://moddingwiki.shikadi.net/wiki/ART_Format_\(Build\))

##  Sample files

  * [Boot Disc Issue 01](https://archive.org/details/cdrom-boot-disc-01) → CONTENT/DEMOS/GAMES/DUKE3D/DUKE3D.GRP ([GRP](/wiki/GRP_\(Duke_Nukem_3D\) "GRP \(Duke Nukem 3D\)") format) → TILES*.ART (and PALETTE.DAT) 

Retrieved from
"[http://fileformats.archiveteam.org/index.php?title=ART_(Duke_Nukem_3D)&oldid=44594](http://fileformats.archiveteam.org/index.php?title=ART_\(Duke_Nukem_3D\)&oldid=44594)"

[Categories](/wiki/Special:Categories "Special:Categories"):

  * [File Formats](/wiki/Category:File_Formats "Category:File Formats")
  * [Electronic File Formats](/wiki/Category:Electronic_File_Formats "Category:Electronic File Formats")
  * [Graphics](/wiki/Category:Graphics "Category:Graphics")
  * [File formats with extension .art](/wiki/Category:File_formats_with_extension_.art "Category:File formats with extension .art")
  * [Build Engine formats](/wiki/Category:Build_Engine_formats "Category:Build Engine formats")
  * [Game data files](/wiki/Category:Game_data_files "Category:Game data files")

Hidden category:

  * [FormatInfo without mimetypes](/wiki/Category:FormatInfo_without_mimetypes "Category:FormatInfo without mimetypes")

##### Personal tools

  * [Log in / create account](/index.php?title=Special:UserLogin&returnto=ART+%28Duke+Nukem+3D%29 "You are encouraged to log in; however, it is not mandatory \[o\]")

##### Namespaces

  * [Page](/wiki/ART_\(Duke_Nukem_3D\) "View the content page \[c\]")
  * [Discussion](/index.php?title=Talk:ART_\(Duke_Nukem_3D\)&action=edit&redlink=1 "Discussion about the content page \[t\]")

####

##### Variants

##### Views

  * [Read](/wiki/ART_\(Duke_Nukem_3D\))
  * [View source](/index.php?title=ART_\(Duke_Nukem_3D\)&action=edit "This page is protected.
You can view its source \[e\]")

  * [View history](/index.php?title=ART_\(Duke_Nukem_3D\)&action=history "Past revisions of this page \[h\]")

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

  * [What links here](/wiki/Special:WhatLinksHere/ART_\(Duke_Nukem_3D\) "A list of all wiki pages that link here \[j\]")
  * [Related changes](/wiki/Special:RecentChangesLinked/ART_\(Duke_Nukem_3D\) "Recent changes in pages linked from this page \[k\]")
  * [Special pages](/wiki/Special:SpecialPages "A list of all special pages \[q\]")
  * [Printable version](/index.php?title=ART_\(Duke_Nukem_3D\)&printable=yes)
  * [Permanent link](/index.php?title=ART_\(Duke_Nukem_3D\)&oldid=44594 "Permanent link to this revision of the page")

  * This page was last modified on 27 June 2023, at 20:07.
  * This page has been accessed 11,025 times.
  * Content is available under [Creative Commons 0](http://creativecommons.org/publicdomain/zero/1.0/).

  * [Privacy policy](/wiki/Just_Solve_the_File_Format_Problem:Privacy_policy "Just Solve the File Format Problem:Privacy policy")
  * [About Just Solve the File Format Problem](/wiki/Just_Solve_the_File_Format_Problem:About "Just Solve the File Format Problem:About")
  * [Disclaimers](/wiki/Just_Solve_the_File_Format_Problem:General_disclaimer "Just Solve the File Format Problem:General disclaimer")

  * [![Creative Commons 0](http://www.mediawiki.org/w/skins/common/images/cc-0.png)](http://creativecommons.org/publicdomain/zero/1.0/)
  * [![Powered by MediaWiki](/skins/common/images/poweredby_mediawiki_88x31.png)](//www.mediawiki.org/)


#!/usr/bin/env bats

# Basic CLI tests for art2img v2

load "common"

@test "CLI shows help when no arguments provided" {
    run ./build/bin/art2img_cli --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "art2img v2.0" ]]
    [[ "$output" =~ "Duke Nukem 3D ART File Converter" ]]
}

@test "CLI shows version" {
    run ./build/bin/art2img_cli --version
    [ "$status" -eq 0 ]
    [[ "$output" =~ "art2img v2.0" ]]
}

@test "CLI fails with missing input file" {
    run ./build/bin/art2img_cli nonexistent.art
    [ "$status" -eq 1 ]
    [[ "$output" =~ "Error: Input path does not exist" ]]
}

@test "CLI fails with missing palette" {
    run ./build/bin/art2img_cli repository/legacy/tests/assets/TILES000.ART
    [ "$status" -eq 1 ]
    [[ "$output" =~ "Error: Palette file does not exist" ]]
}

@test "CLI converts single ART file with palette" {
    # Create temporary output directory
    temp_output=$(mktemp -d)
    
    # Run conversion
    run ./build/bin/art2img_cli repository/legacy/tests/assets/TILES000.ART \
        -p repository/legacy/tests/assets/PALETTE.DAT \
        -o "$temp_output" \
        -f png \
        -v
    
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Processing" ]]
    
    # Check that output files were created
    [ -f "$temp_output/TILES000_0000.png" ]
    
    # Clean up
    rm -rf "$temp_output"
}

@test "CLI converts single ART file to different formats" {
    for format in png tga bmp; do
        temp_output=$(mktemp -d)
        
        run ./build/bin/art2img_cli repository/legacy/tests/assets/TILES000.ART \
            -p repository/legacy/tests/assets/PALETTE.DAT \
            -o "$temp_output" \
            -f "$format" \
            -q
        
        [ "$status" -eq 0 ]
        [ -f "$temp_output/TILES000_0000.$format" ]
        
        rm -rf "$temp_output"
    done
}

@test "CLI processes directory of ART files" {
    temp_output=$(mktemp -d)
    
    # Create a temp directory with some ART files
    temp_art_dir=$(mktemp -d)
    cp repository/legacy/tests/assets/TILES000.ART "$temp_art_dir/"
    cp repository/legacy/tests/assets/TILES001.ART "$temp_art_dir/"
    cp repository/legacy/tests/assets/PALETTE.DAT "$temp_art_dir/"
    
    run ./build/bin/art2img_cli "$temp_art_dir" \
        -o "$temp_output" \
        -f png \
        -v
    
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Found 2 ART files" ]]
    
    # Check that output files were created for both ART files
    [ -f "$temp_output/TILES000_0000.png" ]
    [ -f "$temp_output/TILES001_0000.png" ]
    
    # Clean up
    rm -rf "$temp_output" "$temp_art_dir"
}

@test "CLI respects threading options" {
    temp_output=$(mktemp -d)
    
    # Test sequential processing
    run ./build/bin/art2img_cli repository/legacy/tests/assets/TILES000.ART \
        -p repository/legacy/tests/assets/PALETTE.DAT \
        -o "$temp_output" \
        -f png \
        --no-parallel \
        -q
    
    [ "$status" -eq 0 ]
    [ -f "$temp_output/TILES000_0000.png" ]
    
    # Test parallel processing with specific job count
    run ./build/bin/art2img_cli repository/legacy/tests/assets/TILES001.ART \
        -p repository/legacy/tests/assets/PALETTE.DAT \
        -o "$temp_output" \
        -f png \
        -j 2 \
        -q
    
    [ "$status" -eq 0 ]
    [ -f "$temp_output/TILES001_0000.png" ]
    
    rm -rf "$temp_output"
}

@test "CLI rejects invalid format" {
    run ./build/bin/art2img_cli repository/legacy/tests/assets/TILES000.ART \
        -p repository/legacy/tests/assets/PALETTE.DAT \
        -f invalid_format
    
    [ "$status" -eq 1 ]
    [[ "$output" =~ "Unsupported format" ]]
}
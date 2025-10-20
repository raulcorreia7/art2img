# CI Pipeline Troubleshooting Guide

## Overview

This document provides comprehensive troubleshooting procedures for the art2img CI pipeline across all supported platforms (Ubuntu, macOS, Windows).

## Platform-Specific Issues

### Ubuntu (Linux)

#### Common Issues
1. **Missing Dependencies**
   - **Symptoms**: Build fails with missing cmake, lcov, or ninja
   - **Solution**: Ensure apt-get update runs successfully
   - **Commands**: `sudo apt-get update && sudo apt-get install -y cmake lcov ninja-build`

2. **Permission Issues**
   - **Symptoms**: Permission denied during build or test
   - **Solution**: Check file permissions in build directory
   - **Commands**: `chmod -R 755 build/`

3. **Coverage Generation Failures**
   - **Symptoms**: Codecov upload fails
   - **Solution**: Check if coverage.info exists and is valid
   - **Commands**: `lcov --remove coverage.info '/usr/*' --output-file coverage.info`

### macOS

#### Common Issues
1. **Homebrew Issues**
   - **Symptoms**: cmake or ninja not found
   - **Solution**: Update Homebrew and reinstall packages
   - **Commands**: `brew update && brew install cmake ninja`

2. **Parallel Build Issues**
   - **Symptoms**: Build hangs or fails with high parallelism
   - **Solution**: Reduce parallelism or use sequential build
   - **Commands**: `cmake --build build --config Debug --parallel 1`

3. **Filesystem Case Sensitivity**
   - **Symptoms**: File not found errors
   - **Solution**: Ensure consistent case in file references
   - **Check**: Use `ls -la` to verify exact filenames

### Windows

#### Common Issues
1. **PowerShell Execution Policy**
   - **Symptoms**: Scripts blocked from running
   - **Solution**: Set appropriate execution policy
   - **Commands**: `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser`

2. **Path Issues**
   - **Symptoms**: Executables not found
   - **Solution**: Check PATH environment variable
   - **Commands**: `echo $env:PATH`

3. **Architecture Mismatches**
   - **Symptoms**: Build fails for specific architecture
   - **Solution**: Ensure correct -A flag for CMake
   - **Check**: Verify matrix.arch matches target platform

## Test Failures

### General Test Debugging

1. **Run Tests Locally**
   ```bash
   # Linux/macOS
   cd build && ctest --output-on-failure --timeout 300
   
   # Windows
   cd build; ctest --output-on-failure --timeout 300
   ```

2. **Run Specific Test**
   ```bash
   # Linux/macOS
   cd build/linux_x64 && ./tests/art2img_tests --test-case="test_name"
   
   # Windows
   cd build\windows_x64 && .\tests\art2img_tests.exe --test-case="test_name"
   ```

3. **Verbose Test Output**
   ```bash
   ./tests/art2img_tests --success --no-skip
   ```

### Platform-Specific Test Issues

1. **Timing Issues**
   - **Symptoms**: Tests timeout on slower platforms
   - **Solution**: Increase timeout values
   - **Location**: CMakeLists.txt test configuration

2. **Asset Loading Issues**
   - **Symptoms**: Test assets not found
   - **Solution**: Verify TEST_ASSET_SOURCE_DIR path
   - **Check**: Ensure assets directory exists in test location

3. **Parallel Test Execution**
   - **Symptoms**: Tests fail when run in parallel
   - **Solution**: Reduce parallelism or run sequentially
   - **Commands**: `ctest --parallel 1`

## Build Issues

### CMake Configuration

1. **Cache Issues**
   - **Symptoms**: Stale configuration causing build failures
   - **Solution**: Clear CMake cache
   - **Commands**: `rm -rf build/ && cmake -S . -B build`

2. **Dependency Issues**
   - **Symptoms**: CPM cache dirty or corrupted
   - **Solution**: Clear CPM cache
   - **Commands**: `rm -rf .cache/`

3. **Generator Issues**
   - **Symptoms**: Build system generation fails
   - **Solution**: Try different generators
   - **Commands**: 
     ```bash
     # Unix
     cmake -S . -B build -G "Unix Makefiles"
     
     # Windows
     cmake -S . -B build -G "Visual Studio 17 2022"
     ```

## Rollback Procedures

### Emergency Rollback

1. **Revert to Last Working Commit**
   ```bash
   git log --oneline -10  # Find last working commit
   git revert <commit_hash>  # Revert problematic commit
   git push origin main
   ```

2. **Hotfix Rollback**
   ```bash
   # Create hotfix branch
   git checkout -b hotfix/ci-rollback
   git revert HEAD~1  # Revert last commit
   git push origin hotfix/ci-rollback
   # Create PR for hotfix
   ```

### Configuration Rollback

1. **CI Configuration**
   - **File**: `.github/workflows/ci.yml`
   - **Backup**: Keep previous version in git history
   - **Rollback**: `git checkout HEAD~1 -- .github/workflows/ci.yml`

2. **CMake Configuration**
   - **Files**: `CMakeLists.txt`, `tests/CMakeLists.txt`
   - **Rollback**: Revert specific changes using git

## Performance Optimization

### Build Performance

1. **Cache Optimization**
   - Use CPM cache effectively
   - Enable ccache if available
   - Parallel builds where appropriate

2. **Test Performance**
   - Run tests in parallel where safe
   - Use appropriate timeout values
   - Skip expensive tests on CI when possible

### CI Pipeline Optimization

1. **Matrix Optimization**
   - Use fail-fast: false for better debugging
   - Platform-specific optimizations
   - Conditional steps to reduce runtime

## Monitoring and Alerting

### Key Metrics

1. **Build Time**
   - Target: < 10 minutes per platform
   - Alert: > 15 minutes

2. **Test Execution Time**
   - Target: < 5 minutes per platform
   - Alert: > 10 minutes

3. **Success Rate**
   - Target: > 95% success rate
   - Alert: < 90% success rate

### Log Analysis

1. **Common Failure Patterns**
   - Dependency installation failures
   - Build configuration issues
   - Test timeout failures

2. **Debug Information**
   - Always use --output-on-failure
   - Enable verbose logging for debugging
   - Capture system information on failure

## Contact and Support

### Escalation Procedures

1. **Platform-Specific Issues**
   - Ubuntu: Check system packages and dependencies
   - macOS: Verify Homebrew installation
   - Windows: Check PowerShell and Visual Studio setup

2. **Infrastructure Issues**
   - GitHub Actions status: https://www.githubstatus.com/
   - Dependency availability: Check package repositories

### Documentation Updates

1. **Update This Document**
   - Add new issues as they are discovered
   - Update solutions as they are found
   - Maintain platform-specific sections

2. **Version Control**
   - Tag troubleshooting guide versions
   - Link to specific CI pipeline versions
   - Maintain change log for procedures
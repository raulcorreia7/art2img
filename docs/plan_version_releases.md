# Version Releases Implementation Plan

## Current State Analysis

### Existing Version Infrastructure
- **Version location**: Hardcoded in `Makefile:6` as `VERSION := 1.0.0`
- **CLI version**: Hardcoded in `src/art2img.cpp:252` as `"art2img 1.0.0"`
- **CI integration**: GitHub Actions extracts version from Makefile for release naming
- **Release workflow**: Already exists in `.github/workflows/build.yml` with release job
- **Release assets**: Automatically creates platform-specific packages (tar.gz/zip)

### Current Gaps
- No automated version bumping mechanism
- No CHANGELOG maintenance process
- Version inconsistency risk between Makefile and source code
- No semantic versioning enforcement
- No release notes generation beyond template

## Version Management Strategy

### Semantic Versioning Policy
- **Format**: `MAJOR.MINOR.PATCH` (e.g., `1.0.0`)
- **MAJOR**: Breaking changes (API changes, file format changes)
- **MINOR**: New features (new output formats, new CLI options)
- **PATCH**: Bug fixes (memory leaks, crash fixes, build fixes)

### Version Sources of Truth
1. **Primary**: `Makefile` - Single source for version number
2. **Derived**: All other locations auto-sync from Makefile
3. **Validation**: CI ensures consistency across all locations

## Implementation Plan

### Phase 1: Version Synchronization
**Goal**: Ensure single source of truth for version information

#### Tasks:
1. **Create version header** (`include/version.hpp`)
   - Auto-generated from Makefile during build
   - Provides `ART2IMG_VERSION` macro for source code
   - Includes version parsing utilities

2. **Update source code version usage**
   - Replace hardcoded version in `src/art2img.cpp`
   - Use generated version header throughout codebase

3. **Add version validation target**
   - `make verify-version` checks consistency
   - CI integration for version validation

### Phase 2: Automated Version Bumping
**Goal**: Streamlined version increment process

#### Tasks:
1. **Create version bump script** (`scripts/bump-version.sh`)
   - Supports `major`, `minor`, `patch` increments
   - Updates Makefile and creates git tag
   - Generates commit with conventional commit format

2. **Add Makefile version targets**
   ```makefile
   bump-patch:  # Increment patch version
   bump-minor:  # Increment minor version  
   bump-major:  # Increment major version
   ```

3. **GitHub Actions workflow for version bumps**
   - Triggered by manual dispatch
   - Runs version bump script
   - Creates and pushes tag

### Phase 3: CHANGELOG Management
**Goal**: Automated changelog generation and maintenance

#### Tasks:
1. **Initialize CHANGELOG.md** with standardized format
   - Keep a Changelog format
   - Sections for Unreleased, Added, Changed, Deprecated, Removed, Fixed, Security

2. **Create changelog generation script** (`scripts/generate-changelog.sh`)
   - Parses git commits since last tag
   - Categorizes changes by conventional commit types
   - Updates CHANGELOG.md

3. **Integrate with release workflow**
   - Auto-generate changelog for releases
   - Include in GitHub release notes

### Phase 4: Enhanced Release Automation
**Goal**: Fully automated release process with rich metadata

#### Tasks:
1. **Enhanced release notes generation**
   - Auto-generated changelog section
   - Build matrix summary
   - Binary checksums and verification info
   - Installation instructions per platform

2. **Release validation workflow**
   - Post-release smoke tests
   - Binary integrity verification
   - Documentation link validation

3. **Release promotion workflow**
   - Support for pre-release/beta releases
   - Automatic promotion to stable based on criteria

## Technical Implementation Details

### Version Header Generation
```makefile
# Generate version header from Makefile
$(INCLUDEDIR)/version.hpp: Makefile
	@echo "Generating version header..."
	@mkdir -p $(INCLUDEDIR)
	@echo "#pragma once" > $@
	@echo "#define ART2IMG_VERSION \"$(VERSION)\"" >> $@
	@echo "#define ART2IMG_VERSION_MAJOR $(shell echo $(VERSION) | cut -d. -f1)" >> $@
	@echo "#define ART2IMG_VERSION_MINOR $(shell echo $(VERSION) | cut -d. -f2)" >> $@
	@echo "#define ART2IMG_VERSION_PATCH $(shell echo $(VERSION) | cut -d. -f3)" >> $@
```

### Version Bump Script Structure
```bash
#!/bin/bash
# scripts/bump-version.sh
set -euo pipefail

BUMP_TYPE=${1:-patch}
CURRENT_VERSION=$(grep "VERSION :=" Makefile | cut -d' ' -f3)

# Version bumping logic
# Git tag creation
# Commit creation with conventional format
```

### Enhanced Release Workflow
- **Trigger**: Git tag push matching `v*.*.*` pattern
- **Validation**: Version consistency checks
- **Build**: Multi-platform binary creation
- **Testing**: Comprehensive test suite
- **Packaging**: Platform-specific archives with metadata
- **Release**: GitHub release with auto-generated notes
- **Verification**: Post-release smoke tests

## Risk Mitigation

### Version Consistency
- **Risk**: Version numbers diverging between files
- **Mitigation**: Single source of truth, validation targets, CI checks

### Release Automation
- **Risk**: Automated releases with broken builds
- **Mitigation**: Comprehensive test suite, staging releases, manual approval gates

### Changelog Accuracy
- **Risk**: Inaccurate or incomplete changelog
- **Mitigation**: Conventional commit enforcement, automated generation, manual review

## Success Metrics

1. **Version consistency**: 100% across all files
2. **Release automation**: Fully automated from tag to release
3. **Changelog quality**: Complete change tracking for each release
4. **Release reliability**: Zero failed releases due to version issues
5. **Developer experience**: Simple version bump commands with clear feedback

## Implementation Timeline

- **Phase 1**: 1-2 days (version synchronization)
- **Phase 2**: 2-3 days (automated bumping)
- **Phase 3**: 2-3 days (changelog management)
- **Phase 4**: 3-4 days (enhanced automation)

**Total estimated effort**: 8-12 days

## Dependencies

- **Tools**: Standard Unix utilities (sed, awk, git)
- **GitHub Actions**: Existing workflow enhancement
- **No new external dependencies** required

## Rollback Strategy

Each phase can be independently rolled back:
- Revert version header generation
- Restore hardcoded version values
- Disable automated workflows
- Use manual release process as fallback
# Simple Version Releases Plan

## Current State
- Version hardcoded in Makefile: `VERSION := 1.0.0`
- Version hardcoded in src/art2img.cpp: `"art2img 1.0.0"`
- GitHub Actions already builds releases
- No automated version bumping
- No CHANGELOG

## Simple Implementation

### 1. Version Synchronization
**Goal**: Makefile the single source of truth

**Changes:**
- Generate `include/version.hpp` from Makefile during build
- Update `src/art2img.cpp` to use generated header
- Add `make verify-version` target

### 2. Basic Version Bumping
**Goal**: Simple script to increment versions

**Script**: `scripts/bump-version.sh`
```bash
#!/bin/bash
# Usage: ./scripts/bump-version.sh [patch|minor|major]
```

**Makefile targets:**
```makefile
bump-patch:
	./scripts/bump-version.sh patch

bump-minor:  
	./scripts/bump-version.sh minor

bump-major:
	./scripts/bump-version.sh major
```

### 3. Simple CHANGELOG
**Goal**: Basic changelog for releases

**Create**: `CHANGELOG.md` with standard format
```markdown
# Changelog

## [Unreleased]
- Changes go here

## [1.0.0] - 2024-XX-XX
- Initial release
```

### 4. Enhanced Release Workflow
**Goal**: Better release notes from existing workflow

**Changes to `.github/workflows/build.yml`:**
- Add changelog section to release notes
- Include version in release name
- Add basic validation

## Implementation Order
1. Version synchronization (30 min)
2. Version bump script (1 hour)
3. CHANGELOG setup (30 min)
4. Release workflow enhancement (1 hour)

**Total: ~3 hours**

## Files to Create/Modify
- `include/version.hpp` (generated)
- `src/art2img.cpp` (version usage)
- `Makefile` (version targets)
- `scripts/bump-version.sh` (new)
- `CHANGELOG.md` (new)
- `.github/workflows/build.yml` (enhance)

## Success Criteria
- Version defined only in Makefile
- `make bump-patch` works
- CHANGELOG exists and is updated
- Releases include version info
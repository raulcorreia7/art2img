# 01 Version Synchronization

## Intent
Establish single source of truth for version information across the entire codebase to eliminate version inconsistency issues.

## Scope
**Files to modify:**
- `Makefile` - Add version header generation
- `include/version.hpp` - New auto-generated version header
- `src/art2img.cpp` - Replace hardcoded version
- `.github/workflows/build.yml` - Add version validation

**Out of scope:**
- Version bumping automation (covered in task 02)
- CHANGELOG generation (covered in task 03)
- Release workflow enhancements (covered in task 04)

## Acceptance Criteria
- [ ] Version defined only in Makefile
- [ ] Auto-generated version header during build
- [ ] All source code uses version header
- [ ] Version validation make target
- [ ] CI validates version consistency
- [ ] No hardcoded versions in source code

## Plan
1. **Create version header generation rule in Makefile**
   - Add target to generate `include/version.hpp`
   - Extract version components (major, minor, patch)
   - Make header generation dependency for compilation

2. **Update source code to use version header**
   - Replace hardcoded version in `src/art2img.cpp`
   - Include generated version header
   - Use `ART2IMG_VERSION` macro

3. **Add version validation**
   - Create `make verify-version` target
   - Check version consistency across files
   - Validate version format compliance

4. **Update CI workflow**
   - Add version validation step
   - Fail build if versions inconsistent
   - Report version in build logs

## Tests
- **Unit test**: Version header generation produces correct format
- **Integration test**: Version validation catches inconsistencies
- **Build test**: Compilation succeeds with generated header
- **CI test**: Workflow validates version correctly

## Risks & Rollback
**Risks:**
- Build failure due to missing header generation
- Circular dependencies in Makefile
- Version format parsing errors

**Rollback:**
- Revert to hardcoded version in source
- Remove version header generation
- Disable version validation in CI

## Evidence
- Version header generation working
- All tests passing with new version system
- CI validating versions successfully
- No hardcoded versions remaining

## Status
Owner: Agent • State: Todo • Updated: 2025-10-02
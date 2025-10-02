# 05 Version Validation

## Intent
Implement comprehensive version validation and consistency checks across the entire project to prevent version-related issues.

## Scope
**Files to create:**
- `scripts/validate-version.sh` - Version validation script

**Files to modify:**
- `Makefile` - Add validation targets
- `.github/workflows/build.yml` - Add validation steps
- `tests/test_version_validation.cpp` - Version validation tests

**Out of scope:**
- Version synchronization (covered in task 01)
- Version bumping (covered in task 02)
- CHANGELOG management (covered in task 03)
- Release automation (covered in task 04)

## Acceptance Criteria
- [ ] Version format validation (semantic versioning)
- [ ] Consistency checks across all version locations
- [ ] Git tag validation against Makefile version
- [ ] CLI version output validation
- [ ] Binary version embedding verification
- [ ] Comprehensive test coverage
- [ ] CI integration with failure reporting

## Plan
1. **Create version validation script**
   - Validate semantic version format
   - Check version consistency across files:
     - Makefile VERSION
     - Generated version header
     - CLI version output
     - Git tag format
   - Verify version components are numeric
   - Check for version conflicts

2. **Add Makefile validation targets**
   ```makefile
   validate-version:  # Comprehensive version validation
   check-version-format:  # Semantic version format check
   verify-version-consistency:  # Cross-file consistency
   ```

3. **Create version validation tests**
   - Test version format validation
   - Test consistency checking
   - Test version parsing utilities
   - Test error handling for invalid versions
   - Test CLI version output

4. **Integrate with CI workflows**
   - Add validation to build workflow
   - Fail fast on version inconsistencies
   - Report detailed validation results
   - Add validation to release workflow

5. **Add binary version verification**
   - Check version string in compiled binaries
   - Verify version embedding works correctly
   - Test version output from CLI

## Tests
- **Format test**: Validates semantic version format
- **Consistency test**: Detects version mismatches
- **CLI test**: Verifies version command output
- **Binary test**: Checks version in compiled binaries
- **Git test**: Validates tag format and content
- **Error test**: Handles invalid versions gracefully

## Risks & Rollback
**Risks:**
- False positive validation failures
- Version format too restrictive
- Performance impact on builds
- Complex validation logic

**Rollback:**
- Simplify validation rules
- Disable strict validation
- Use basic version checks
- Manual verification process

## Evidence
- All validation tests pass
- CI catches version inconsistencies
- Version format compliance verified
- Binary version embedding confirmed
- No false positive failures

## Status
Owner: Agent • State: Todo • Updated: 2025-10-02
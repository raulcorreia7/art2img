# 02 Automated Version Bumping

## Intent
Implement streamlined version increment process with proper git tagging and conventional commit formatting.

## Scope
**Files to create:**
- `scripts/bump-version.sh` - Version bump automation script

**Files to modify:**
- `Makefile` - Add version bump targets
- `.github/workflows/bump-version.yml` - New workflow for version bumps

**Out of scope:**
- Version synchronization (covered in task 01)
- CHANGELOG generation (covered in task 03)
- Release automation (covered in task 04)

## Acceptance Criteria
- [ ] Version bump script supports major/minor/patch increments
- [ ] Makefile provides bump-patch, bump-minor, bump-major targets
- [ ] Script creates git tag and conventional commit
- [ ] GitHub Actions workflow for automated bumping
- [ ] Version validation after bumping
- [ ] Proper error handling and rollback capability

## Plan
1. **Create version bump script**
   - Parse current version from Makefile
   - Implement semantic version increment logic
   - Update Makefile with new version
   - Create git tag with format `vX.Y.Z`
   - Generate conventional commit message
   - Push changes and tag to remote

2. **Add Makefile version bump targets**
   ```makefile
   bump-patch:  # Increment patch version
   bump-minor:  # Increment minor version
   bump-major:  # Increment major version
   ```

3. **Create GitHub Actions workflow**
   - Manual dispatch trigger
   - Run version bump script
   - Validate new version
   - Push changes to repository

4. **Add validation and error handling**
   - Verify version format before bumping
   - Check for uncommitted changes
   - Validate git tag creation
   - Provide rollback instructions

## Tests
- **Script test**: Version bump increments correctly
- **Git test**: Proper tag and commit creation
- **Error test**: Handles invalid inputs gracefully
- **Workflow test**: GitHub Actions executes successfully
- **Rollback test**: Can revert failed bump

## Risks & Rollback
**Risks:**
- Version format corruption
- Git tag conflicts
- Push failures leaving inconsistent state
- Conventional commit format errors

**Rollback:**
- Reset to previous commit
- Delete created git tag
- Restore Makefile from git
- Manual version correction process

## Evidence
- Script successfully bumps all version types
- Git history shows proper conventional commits
- Tags created with correct format
- GitHub Actions workflow completes
- Error scenarios handled gracefully

## Status
Owner: Agent • State: Todo • Updated: 2025-10-02
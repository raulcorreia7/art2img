# CI Pipeline Rollback Procedures

## Overview

This document provides step-by-step rollback procedures for CI pipeline changes. These procedures ensure quick recovery from failed deployments or configuration issues.

## Emergency Rollback Checklist

### Immediate Actions (First 5 minutes)

1. **Assess Impact**
   - [ ] Check CI status on GitHub
   - [ ] Identify affected platforms
   - [ ] Determine failure scope (build/test/config)

2. **Communicate Status**
   - [ ] Notify team of CI issues
   - [ ] Update project status if needed
   - [ ] Document start time of rollback

3. **Prepare Rollback Environment**
   - [ ] Ensure git access is available
   - [ ] Verify permissions for CI configuration
   - [ ] Backup current state if needed

## Complete Rollback Procedures

### Procedure 1: Full CI Configuration Rollback

**When to Use**: Complete CI pipeline failure affecting all platforms

**Steps**:
1. **Identify Last Working Commit**
   ```bash
   git log --oneline --grep="CI" -10
   # Look for last commit with "CI" that was working
   ```

2. **Create Rollback Branch**
   ```bash
   git checkout -b rollback/ci-emergency
   git reset --hard <last_working_commit_hash>
   ```

3. **Verify Rollback**
   ```bash
   git diff HEAD~1 .github/workflows/
   # Confirm changes are reverted
   ```

4. **Apply Rollback**
   ```bash
   git push origin rollback/ci-emergency --force
   # Create PR to merge rollback branch
   ```

5. **Update Main Branch**
   ```bash
   git checkout main
   git merge rollback/ci-emergency
   git push origin main
   ```

### Procedure 2: Platform-Specific Rollback

**When to Use**: Single platform failure (Ubuntu/macOS/Windows)

**Steps**:
1. **Isolate Platform Configuration**
   ```bash
   # Check platform-specific changes
   git diff HEAD~1 .github/workflows/ci.yml | grep -A5 -B5 "ubuntu\|macos\|windows"
   ```

2. **Partial Rollback**
   ```bash
   # Create patch for specific platform
   git show <last_working_commit_hash>: .github/workflows/ci.yml > temp_ci.yml
   # Edit temp_ci.yml to fix only the problematic platform
   mv temp_ci.yml .github/workflows/ci.yml
   ```

3. **Test and Deploy**
   ```bash
   git add .github/workflows/ci.yml
   git commit -m "hotfix: rollback platform-specific CI configuration"
   git push origin main
   ```

### Procedure 3: Test Configuration Rollback

**When to Use**: Test failures but builds succeed

**Steps**:
1. **Identify Test Configuration Changes**
   ```bash
   git log --oneline --grep="test" -5
   git diff HEAD~1 tests/ CMakeLists.txt
   ```

2. **Rollback Test Configuration**
   ```bash
   git checkout HEAD~1 -- tests/CMakeLists.txt
   git checkout HEAD~1 -- CMakeLists.txt
   ```

3. **Verify Test Configuration**
   ```bash
   # Local test verification
   rm -rf build/
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
   cmake --build build
   cd build && ctest --output-on-failure
   ```

4. **Deploy Rollback**
   ```bash
   git add tests/CMakeLists.txt CMakeLists.txt
   git commit -m "hotfix: rollback test configuration"
   git push origin main
   ```

### Procedure 4: Dependency Rollback

**When to Use**: CPM cache or dependency issues

**Steps**:
1. **Clear Dependency Cache**
   ```bash
   # In CI workflow, add cache clearing step
   - name: Clear CPM Cache
     run: rm -rf .cache/
   ```

2. **Rollback Dependency Versions**
   ```bash
   # Check CMakeLists.txt dependency versions
   git diff HEAD~1 CMakeLists.txt | grep -A3 -B3 "CPMAddPackage\|VERSION"
   
   # Revert to working versions
   git checkout HEAD~1 -- CMakeLists.txt
   ```

3. **Force Dependency Refresh**
   ```bash
   # Add to CI workflow
   - name: Force Dependency Refresh
     run: |
       rm -rf .cache/
       cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
   ```

## Quick Rollback Commands

### Git-Based Rollbacks

```bash
# 1. Soft reset (keep changes)
git reset --soft HEAD~1

# 2. Hard reset (lose changes)
git reset --hard HEAD~1

# 3. Revert specific commit
git revert <commit_hash>

# 4. Revert specific file
git checkout HEAD~1 -- path/to/file

# 5. Create rollback branch
git checkout -b rollback/emergency
git reset --hard <working_commit>
```

### CI-Specific Rollbacks

```bash
# 1. Rollback CI configuration only
git checkout HEAD~1 -- .github/workflows/

# 2. Rollback CMake configuration
git checkout HEAD~1 -- CMakeLists.txt */CMakeLists.txt

# 3. Rollback test configuration
git checkout HEAD~1 -- tests/

# 4. Clear caches and restart
rm -rf build/ .cache/
git clean -fd
```

## Verification Procedures

### Pre-Rollback Verification

1. **Backup Current State**
   ```bash
   git tag backup-before-rollback-$(date +%Y%m%d-%H%M%S)
   git push origin backup-before-rollback-*
   ```

2. **Document Current Issues**
   ```bash
   # Capture current CI status
   gh run list --limit 5
   # Document specific error messages
   ```

### Post-Rollback Verification

1. **Local Testing**
   ```bash
   # Full build and test cycle
   make clean
   make all
   make test
   ```

2. **CI Pipeline Testing**
   ```bash
   # Trigger CI run
   git commit --allow-empty -m "trigger: test CI rollback"
   git push origin main
   ```

3. **Platform Verification**
   - [ ] Ubuntu builds and tests pass
   - [ ] macOS builds and tests pass  
   - [ ] Windows x64 builds and tests pass
   - [ ] Windows x86 builds and tests pass

## Communication Procedures

### Rollback Announcement Template

```
Subject: [EMERGENCY] CI Pipeline Rollback Completed

Status: ROLLBACK COMPLETED
Time: YYYY-MM-DD HH:MM UTC
Affected Platforms: [Ubuntu/macOS/Windows/All]
Root Cause: [Brief description]
Actions Taken: [Rollback procedure used]
Current Status: [CI is stable/unstable]
Next Steps: [Further investigation needed?]
```

### Post-Rollback Review

1. **Incident Report**
   - Document timeline of events
   - Identify root cause
   - Record rollback actions taken
   - Note any data loss or corruption

2. **Prevention Measures**
   - Update testing procedures
   - Add additional safeguards
   - Improve monitoring and alerting
   - Update documentation

## Recovery Time Objectives

| Severity | Target Recovery Time | Maximum Downtime |
|----------|---------------------|------------------|
| Critical (all platforms) | 15 minutes | 30 minutes |
| High (single platform) | 30 minutes | 1 hour |
| Medium (test failures) | 1 hour | 2 hours |
| Low (configuration) | 2 hours | 4 hours |

## Escalation Contacts

### Primary Contacts
- **CI Pipeline Owner**: [Contact information]
- **Build System Expert**: [Contact information]
- **Platform Specialists**: 
  - Ubuntu: [Contact]
  - macOS: [Contact]
  - Windows: [Contact]

### Escalation Procedure
1. **Level 1**: Immediate rollback using documented procedures
2. **Level 2**: Engage platform specialists if rollback fails
3. **Level 3**: Escalate to infrastructure team if systemic issues

## Rollback Testing

### Monthly Rollback Drills

1. **Simulate CI Failure**
   - Intentionally break CI configuration
   - Practice rollback procedures
   - Time the recovery process

2. **Validate Procedures**
   - Test all rollback procedures
   - Update documentation based on findings
   - Train team members on procedures

3. **Update Safeguards**
   - Add missing safeguards discovered during drills
   - Improve automation where possible
   - Enhance monitoring and alerting

## Documentation Maintenance

### Version Control
- Tag rollback procedure versions
- Maintain change log
- Link to specific CI pipeline versions

### Regular Updates
- Review procedures quarterly
- Update after major CI changes
- Incorporate lessons learned from incidents
# 02 Version Bump Script

## Intent
Simple script to increment version numbers.

## Script
`scripts/bump-version.sh [patch|minor|major]`

## Features
- Parse current version from Makefile
- Increment based on argument
- Update Makefile
- Create git tag
- Commit changes

## Makefile Targets
```makefile
bump-patch:
	./scripts/bump-version.sh patch
```

## Acceptance
- Script increments versions correctly
- Creates git tag
- Updates Makefile only
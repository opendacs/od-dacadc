# Contributing
Before contributing to this project, please read the guideline below.
# Branches
This repository is divided in two main branches:

* main
* develop

`main` contains the official release. This branch should allways pass the tests and compilations. You should *never* commit or merge to this branch directly. Only pull requests from `develop` branch or hotfix branches are allowed to this branch.

`develop` is the branch from which feature branches are originated. This branch should be treated with the same care as main branch. Commits to this branch should be done by pull requesting feature branches. Only when a new feature is ready and tested, `develop` branch can be combined with the `main` branch.

## Naming conventions

Follow these branch naming conventions to keep the branches organize.

* `u/username/description` All feature branches should have the name of the user creating them and a description of the feature. For example, user `ckometter` is making a contribution to this markdown under the branch `u/ckometter/contributing`.
* `XX-description` If there is an issue open for that feature, add the issue number `XX` on front for easy identification.
* `hotfix-description` When creating a hotfix branch, add `hotfix` on front and label the pull request as a hotfix.

# Workflow

We suggest the workflow below to ensure development runs smoothly without jeopardizing the main project.

1. Clone the project, switch to develop branch and create a new feature branch for your feature. 
```
$ git clone https://github.com/opendacs/od-dacadc.git
$ git fetch
$ git switch develop
$ git checkout -b [name_of_your_branch] develop
``` 
2. Commit changes to your feature branch and push it.
```
$ git add .
$ git commit -m '[your-commit-description]'
$ git push origin [name-of-your-branch]
```
5. When your feature branch is ready to merge to `develop`, create a pull request to review your contribution.

# Coco

Common, generic, or predicted-reusable code for my Qt projects.

```bat
git submodule add https://github.com/fairybow/Coco Project\submodules\Coco
git submodule update --init --recursive
```

```bat
git submodule deinit -f "Project\submodules\Coco"
git rm -f "Project\submodules\Coco"
rmdir /s /q ".git\modules\Project\submodules\Coco"
```

## Configuration

Project Properties > Configuration Properties > General > C++ Language Standard:

```
ISO C++20 Standard (/std:c++20)
```

Project Properties > Configuration Properties > C/C++ > General > Additional Include Directories:

```
$(ProjectDir)submodules\Coco\Coco\include;%(AdditionalIncludeDirectories)
```

# MyProjectName
A [Brief Description]

![](MyProjectName-logo.webp "[Insert a cool phrase]")


This is just a template repository that I use to develop my projects. It targets modern C++ and CMake features, so keeps
yourself aware of it. 

This is an idiosyncratic project, so the choices made here can differ from the community standards. I think that CMake
is a very lenient tool, so to keep my own sanity, I decided to make some constraints. First, I will focus only on making
dependencies constructed with this template safe to consume via CPM and package managers (No C programmers, git
submodules aren't a good package manager). It's recommended that both your primary and third-party code also follows
this template (not just for the code, but to assert that things like data files and LLDB scripts will work). Even raw
find_dependency() isn't recommended.

# What You Need to Change

As a general rule, find and replace all occurrences of "MyProjectName" and "MYPROJECTNAME" with your actual library
name.
This applies to directory names, file names, and file contents.

After that, update these specific cases:

- `..github`: Update to `.github`. 
- `CMakeLists.txt`: Update the `project()` block with your real `VERSION`and `DESCRIPTION`.
- `docs/doxygen/Doxyfile`: Update `PROJECT_NAME`, `PROJECT_BRIEF`, and `PROJECT_LOGO`.
- `docs/doxygen/DoxygenLayout.xml`: Update with your info and page as you need.
- `README.md`: Update the title, brief description, and logo image reference.
- Dependencies: Configure your actual dependencies (like CPMFindPackage) in the `#region linking` section of the root
`CMakeLists.txt`.
- Source files: Place your headers in `include/YourLibraryName/` and sources in `src/YourLibraryName/`, and update the
`target_sources` block accordingly.

the `benchmark` folder isn't mandatory.
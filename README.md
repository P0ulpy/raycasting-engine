# Cmake & vcpkg template

This is a template repository made to quickly set up a project using cmake & vcpkg.
I usually use Clion as IDE so this repository have a basic .idea configuration

You can change the name of the project in the `CMakeLists.txt` and `vcpkg.json` files.

## Bootstrap workspace

*Windows*

Make sure you installed [Visual Studio](https://visualstudio.microsoft.com/) 

Then run the bootstrap script
```bat
.\scripts\bootstrap-workspace.bat
```

*GNU/Linux using apt*

Install necessary build tools and a C/C++ compiler
```sh
sudo apt-get update
sudo apt-get install build-essential tar curl zip unzip autoconf libtool g++ gcc
```

Then run the bootstrap script
```sh
./scripts/bootstrap-workspace.sh
```

Well done your repo is now ready to work with cmake and vcpkg !

## Add dependency

You can add any vcpkg dependencies by editing `vcpkg.json` and then add them to you cmake target with `find_package()` and `target_link_libraries()`. You can found more information about cmake integration in the [vcpkg documentation](https://learn.microsoft.com/vcpkg/users/buildsystems/cmake-integration#using-libraries)

You can found a exemple of adding dependencies in the [Installing dependencies exemple with SFML](#adding-dependencies-exemple-with-sfml-) section.

## Generate your project

Running `generate-cmake-*.sh` will install dependencies if required and then generate your cmake solution.

Use it every times you add a dependency or modify `CMakeLists.txt`

*Windows*
```sh
# For debug build
.\scripts\generate-cmake-debug.bat
# For release build
.\scripts\generate-cmake-release.bat
```

*Unix*
```sh
# For debug build
./scripts/generate-cmake-debug.sh
# For release build
./scripts/generate-cmake-release.sh
```

## Compile your project

Cmake will atomatically detect your compiler and generator 

*Windows*
```sh
# For debug build
.\scripts\build-debug.bat
# For release build
.\scripts\build-release.bat
```

*Unix*
```bash
# For debug build
./scripts/build-debug.sh
# For release build
./scripts/build-release.sh
```

**Run your program**
You can now run the compiled program by looking into `out/Debug` or `out/Release`.
The out directory hierarchy will be different depending on your generator.

For exemple with Make generator in Debug mode run your program like that
```bash
# The executable name will change acording to the value set in add_executable(<name>, ...)
./out/Debug/cmake-vcpkg-template-app
```

## Adding dependencies exemple with SFML :

In this exemple we will add SFML to our project, make it available in our cmake target and then compile a basic exemple program.

**Add dependencies to vcpkg :**

`vcpkg.json`
```json
{
  "name": "cmake-vcpkg-template",
  "version": "0.1.0",
  "builtin-baseline": "6ca56aeb457f033d344a7106cb3f9f1abf8f4e98",
  "dependencies": [
    {
      "name" : "sfml",
      "version>=" : "2.5.1#14"
    }
  ]
}
```

For further information you can check the [microsoft documentation about vcpkg.json](https://learn.microsoft.com/en-us/vcpkg/reference/vcpkg-json) and [vcpkg documentation about manifests dependencies](https://learn.microsoft.com/vcpkg/concepts/manifest-mode)

**Add dependencies to your cmake target :**

Now that we have added our dependencies to vcpkg we can add them to our cmake target.
First we use `find_package()` to find the package in vcpkg, then we use `target_link_libraries()` to link it to our target, It is important to call it after the target creation (witch is `add_executable` in our case).

`CMakeLists.txt`
```diff
...
+ find_package(SFML COMPONENTS system window graphics network CONFIG REQUIRED)

SET(EXECUTABLE_TARGET_NAME cmake-vcpkg-template-app)

add_executable(${EXECUTABLE_TARGET_NAME}
    main.cpp
)

+ target_link_libraries(${EXECUTABLE_TARGET_NAME}
+     PRIVATE sfml-system sfml-network sfml-graphics sfml-window
+ )
...
```

You must reload your cmake project and reset cache in your IDE or directly with the cmake command to make it work.
```sh
./scripts/generate-cmake-debug.sh
```

**In case of issues while running vcpkg install**

Pay attention at this kind of message of vcpkg :
```
-- SFML currently requires the following libraries from the system package manager:
    libudev
    libx11
    libxrandr
    libxcursor
    opengl
You can intall them in your system using apt-get install libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
```

Most of the time install issues with vcpkg are related to a dynamic library who are not installed in you system. Most of the time vcpkg explain directly how to install them. 

**Use SFML in our program to make sure our installation is working :**

For this exemple we will use the [SFML tutorial](https://www.sfml-dev.org/tutorials/2.6/start-linux.php#compiling-a-sfml-program) to make sure our installation is working.
After pasting the exemple code in your `main.cpp` we can now compile our program and run it, you should see a window with a green circle in it and a title "SFML works!".

Congratulations you have successfully installed SFML in your project !
You can now add any other dependencies you want.

**To go deaper into cmake integration using vcpkg you can start by the** [microsoft documentation for cmake integration using libraries](https://learn.microsoft.com/vcpkg/users/buildsystems/cmake-integration#using-libraries)

## Misc.

### Vcpkg triplets

You can found more information about the vcpkg triplets in the [vcpkg documentation about triplets](https://learn.microsoft.com/fr-fr/vcpkg/users/triplets)

### Updating vcpkg submodule

If you need to update vcpkg or you encounter issues with it you can update the vcpkg submodule with :
```bash
git submodule update --remote
```

After finished it should write in stdout something like :
```
Submodule path 'vcpkg': checked out '<new-commit-sha>'
```

Update the builtin-baseline in `vcpkg.json`
```diff
{
    "name" : "cmake-vcpkg-template",
    "version" : "0.1.0",
-   "builtin-baseline" : "4874bea8eb8db9e6610672cccdd6ccd5d55c6f1a",
+   "builtin-baseline" : <new-commit-sha>,
    "dependencies" : []
}
```

And then commit the modification
```bash
git add .
git commit -m "Update vcpkg to it lastest version"
```

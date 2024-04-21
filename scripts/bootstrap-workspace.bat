git submodule sync --recursive
git submodule update --init --recursive --remote --force
%~dp0..\vcpkg\bootstrap-vcpkg.bat -disableMetrics
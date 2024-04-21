target_include_directories(${APP_TARGET_NAME}
    PUBLIC rlImGui
)

file(GLOB_RECURSE rlImGui_files
    "rlImGui/*.h"
    "rlImGui/*.hpp"
    "rlImGui/*.cpp"
)

target_sources(${APP_TARGET_NAME}
    PUBLIC ${rlImGui_files}
)
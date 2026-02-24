cmake_minimum_required(VERSION 3.12)

# ツールチェーンの読み込み（Docker実行時に指定するならここには不要だが、あっても良い）
# set(CMAKE_TOOLCHAIN_FILE ../arm_toolchain.cmake)

project(try-kernel C CXX ASM)

# ターゲット名の定義
set(TARGET_NAME try-kernel)

# 全ソースファイルを列挙（subdirectoryに分けず、ここで一括管理）
add_executable(${TARGET_NAME}
    application/main.c
    boot/boot2.c
    boot/reset_hdr.c
    boot/vector_tbl.c
    # kernel, device などのファイルもここに連ねる
    kernel/syslib.c
    kernel/context.c
    kernel/dispatch.S
)

# インクルードパスの設定
target_include_directories(${TARGET_NAME} PRIVATE include)

# リンカスクリプトの指定
target_link_options(${TARGET_NAME} PRIVATE -T ${CMAKE_SOURCE_DIR}/linker/pico_memmap.ld)

# リンカスクリプト変更時に再ビルドするための依存関係
set_target_properties(${TARGET_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker/pico_memmap.ld)
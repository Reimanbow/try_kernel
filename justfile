# 既定の動作：すべてをビルドする
default: build

# Dockerイメージを焼き上げる
image:
    docker build -t pico-build-env .

# 城（プロジェクト）を築く（ビルド実行）
build:
    docker run --rm -v $(pwd):/workspace pico-build-env \
        bash -c "mkdir -p build && cd build && \
        cmake -DCMAKE_TOOLCHAIN_FILE=../arm_toolchain.cmake .. && \
        make -j$(nproc)"

# 生成物を清める（掃除）
clean:
    rm -rf build/

# 実機へ出陣（picotoolによる書き込み）
# 注意：ELFファイルをそのまま、あるいはUF2に変換して投げ込みまする
flash: build
    openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "program build/try-kernel verify reset exit"
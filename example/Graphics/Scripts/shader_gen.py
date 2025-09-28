"""Compile GLSL shaders to SPIR-V binaries using the Vulkan SDK.

Usage:
    python shader_gen.py <vulkan_sdk_path> <src_directory> <dst_directory> [--keep-output]
"""

import argparse
import os
import subprocess
import sys
from typing import Sequence

from tools.miscs import path_miscs
from tools.miscs import log

def compile_to_spv(src_path: str, dst_path: str, glslc_path: str) -> None:
    for root, dirs, files in os.walk(src_path):
        for file in files:
            if file.endswith(".vert") or file.endswith(".frag"):
                src_file = os.path.join(root, file)
                relative_path = os.path.relpath(root, src_path)
                dst_dir = dst_path if relative_path == '.' else os.path.join(dst_path, relative_path)
                os.makedirs(dst_dir, exist_ok=True)
                dst_file = os.path.join(dst_dir, file + ".spv")

                try:
                    subprocess.run(
                        [glslc_path, src_file, "-o", dst_file],
                        check=True
                    )
                    log.green(f"Compiled: {src_file} -> {dst_file}")
                except subprocess.CalledProcessError as e:
                    log.red(f"Error compiling {src_file}: {e}")
                except FileNotFoundError:
                    log.red(f"Error: {glslc_path} not found. Make sure Vulkan SDK is installed correctly.")
                    sys.exit(1)


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compile GLSL shaders to SPIR-V using Vulkan SDK's glslc executable."
    )
    parser.add_argument(
        "vulkan_sdk",
        help="Path to the Vulkan SDK root directory."
    )
    parser.add_argument(
        "src_directory",
        help="Directory containing source shader files (.vert/.frag)."
    )
    parser.add_argument(
        "dst_directory",
        help="Directory where compiled SPIR-V binaries will be written."
    )
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> None:
    args = parse_args(argv)

    vulkan_sdk_path = os.path.abspath(args.vulkan_sdk)
    src_path = os.path.abspath(args.src_directory)
    dst_path = os.path.abspath(args.dst_directory)

    if not os.path.isdir(vulkan_sdk_path):
        log.red(f"Error: Vulkan SDK directory '{vulkan_sdk_path}' does not exist.")
        sys.exit(1)

    glslc_path = path_miscs.get_glslc_path(vulkan_sdk_path)

    if not os.path.exists(glslc_path):
        log.red(f"Error: glslc not found at {glslc_path}")
        sys.exit(1)

    if not os.path.isdir(src_path):
        log.red(f"Error: Source directory '{src_path}' does not exist.")
        sys.exit(1)

    path_miscs.clear_dir(dst_path)

    try:
        os.makedirs(dst_path, exist_ok=True)
    except Exception as e:
        log.red(f"Error: Unable to create destination directory '{dst_path}': {e}")
        sys.exit(1)

    log.white(f"Shader compile: {src_path} -> {dst_path}")
    compile_to_spv(src_path, dst_path, glslc_path)


if __name__ == "__main__":
    main()
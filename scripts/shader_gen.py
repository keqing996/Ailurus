import os
import sys
import subprocess
from tools.miscs import path_miscs
from tools.miscs import log

def compile_to_spv(src_path: str, dst_path: str):
    for root, dirs, files in os.walk(src_path):
        for file in files:
            if file.endswith(".vert") or file.endswith(".frag"):
                src_file = os.path.join(root, file)
                relative_path = os.path.relpath(root, src_path)
                dst_dir = os.path.join(dst_path, relative_path)
                os.makedirs(dst_dir, exist_ok=True)
                dst_file = os.path.join(dst_dir, file + ".spv")

                try:
                    subprocess.run(
                        ["glslc", src_file, "-o", dst_file],
                        check=True
                    )
                    log.green(f"Compiled: {src_file} -> {dst_file}")
                except subprocess.CalledProcessError as e:
                    log.red(f"Error compiling {src_file}: {e}")
                except FileNotFoundError:
                    log.red("Error: glslc not found. Make sure it is installed and added to PATH.")
                    sys.exit(1)

def main():
    if len(sys.argv) != 3:
        log.red("Usage: python script.py <src_directory> <dst_directory>")
        sys.exit(1)

    src_path = sys.argv[1]
    dst_path = sys.argv[2]

    # Check valid
    if not os.path.isdir(src_path):
        log.red(f"Error: Source directory '{src_path}' does not exist.")
        sys.exit(1)

    if not os.path.isdir(dst_path):
        try:
            os.makedirs(dst_path, exist_ok=True)
        except Exception as e:
            log.red(f"Error: Unable to create destination directory '{dst_path}': {e}")
            sys.exit(1)

    # Clear dst directory
    path_miscs.clear_dir(dst_path)

    # Compile
    log.white(f"Shader compile: {src_path}")
    compile_to_spv(src_path, dst_path)

if __name__ == "__main__":
    main()
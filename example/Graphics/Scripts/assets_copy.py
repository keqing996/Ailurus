"""Copy assets directory to build output directory.

Usage:
    python assets_copy.py --src_directory <src_directory> --dst_directory <dst_directory>
"""

import argparse
import os
import shutil
import sys
from typing import Sequence
from tools.miscs import log

def copy_directory(src, dst):
    """
    Copy the source directory to the destination directory.
    If the destination directory already exists, delete it first.
    """
    # Check if the source directory exists
    if not os.path.exists(src):
        log.red(f"Error: Source directory '{src}' does not exist.")
        sys.exit(1)

    # If the destination directory exists, delete it
    if os.path.exists(dst):
        log.yellow(f"Target directory '{dst}' already exists. Removing it first...")
        try:
            shutil.rmtree(dst)  # Deletes the destination directory and its contents
        except Exception as e:
            log.red(f"Error: Unable to delete '{dst}'. {e}")
            sys.exit(1)

    # Copy the source directory to the destination
    try:
        shutil.copytree(src, dst)  # Recursively copies the source directory
        log.green(f"Successfully copied '{src}' to '{dst}'.")
    except Exception as e:
        log.red(f"Error: Failed to copy '{src}' to '{dst}'. {e}")
        sys.exit(1)

def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Copy assets directory to build output directory."
    )
    parser.add_argument(
        "--src_directory",
        required=True,
        help="Source directory to copy from."
    )
    parser.add_argument(
        "--dst_directory",
        required=True,
        help="Destination directory to copy to."
    )
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> None:
    """
    Main function for the script.
    It expects the source directory and destination directory as command-line arguments.
    """
    args = parse_args(argv)
    
    # Get the source and destination paths from the parsed arguments
    src_path = os.path.abspath(args.src_directory)
    dst_path = os.path.abspath(args.dst_directory)

    # Call the function to perform the directory copy
    log.white(f"Copy: {src_path} -> {dst_path}")
    copy_directory(src_path, dst_path)

if __name__ == "__main__":
    # Execute the main function
    main()
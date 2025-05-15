import os
import shutil
import sys
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

def main():
    """
    Main function for the script.
    It expects the source directory and destination directory as command-line arguments.
    """
    # Ensure the correct number of command-line arguments
    if len(sys.argv) != 3:
        log.red("Usage: python script.py <src_directory> <dst_directory>")
        sys.exit(1)

    # Get the source and destination paths from the command-line arguments
    src_path = sys.argv[1]
    dst_path = sys.argv[2]

    # Call the function to perform the directory copy
    log.white(f"Copy: {src_path} -> {dst_path}")
    copy_directory(src_path, dst_path)

if __name__ == "__main__":
    # Execute the main function
    main()
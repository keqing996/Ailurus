#!/usr/bin/env python3
"""
Script to fix ICD paths for standalone Vulkan app bundle distribution.
This script modifies ICD JSON files to point to libraries within the app bundle.
"""

import os
import json
import argparse
import sys
from pathlib import Path


def fix_icd_file(icd_file_path, framework_path="@executable_path/../Frameworks"):
    """
    Fix a single ICD file to point to bundled Vulkan libraries.
    
    Args:
        icd_file_path: Path to the ICD JSON file
        framework_path: Path to the frameworks directory relative to executable
    """
    try:
        with open(icd_file_path, 'r') as f:
            icd_data = json.load(f)
        
        # Check if this is a valid ICD file
        if 'ICD' not in icd_data:
            print(f"Warning: {icd_file_path} doesn't appear to be a valid ICD file")
            return False
        
        # Get the original library path
        original_path = icd_data['ICD'].get('library_path', '')
        
        # Extract library name from original path
        lib_name = os.path.basename(original_path)
        
        # Create new bundle-relative path
        if lib_name:
            new_path = f"{framework_path}/{lib_name}"
            icd_data['ICD']['library_path'] = new_path
            
            # Ensure the ICD has the correct API version for MoltenVK
            if 'api_version' not in icd_data['ICD']:
                icd_data['ICD']['api_version'] = "1.3.0"
            
            # Ensure file format version is set
            if 'file_format_version' not in icd_data:
                icd_data['file_format_version'] = "1.0.0"
            new_path = f"{framework_path}/{lib_name}"
            icd_data['ICD']['library_path'] = new_path
            
            print(f"Fixed ICD file: {icd_file_path}")
            print(f"  Original path: {original_path}")
            print(f"  New path: {new_path}")
            
            # Write back the modified file
            with open(icd_file_path, 'w') as f:
                json.dump(icd_data, f, indent=2)
            
            return True
        else:
            print(f"Warning: Could not extract library name from {original_path}")
            return False
            
    except Exception as e:
        print(f"Error processing {icd_file_path}: {e}")
        return False


def main():
    parser = argparse.ArgumentParser(description='Fix ICD paths for app bundle distribution')
    parser.add_argument('--icd_dir', required=True, help='Directory containing ICD files')
    parser.add_argument('--framework_path', default='../../../Frameworks',
                       help='Path to frameworks directory (default: ../../../Frameworks)')

    args = parser.parse_args()
    
    icd_dir = Path(args.icd_dir)
    
    if not icd_dir.exists():
        print(f"Error: ICD directory {icd_dir} does not exist")
        sys.exit(1)
    
    if not icd_dir.is_dir():
        print(f"Error: {icd_dir} is not a directory")
        sys.exit(1)
    
    print(f"Processing ICD files in: {icd_dir}")
    print(f"Framework path: {args.framework_path}")
    
    # Process all JSON files in the ICD directory
    json_files = list(icd_dir.glob('*.json'))
    
    if not json_files:
        print("Warning: No JSON files found in ICD directory")
        return
    
    success_count = 0
    for json_file in json_files:
        if fix_icd_file(json_file, args.framework_path):
            success_count += 1
    
    print(f"\nProcessed {len(json_files)} ICD files, {success_count} successfully modified")
    
    if success_count == 0:
        print("Warning: No ICD files were successfully modified")
        sys.exit(1)


if __name__ == '__main__':
    main()
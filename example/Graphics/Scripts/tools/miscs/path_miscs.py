import os
import shutil
from pathlib import Path


def clear_dir(directory_path: str) -> None:
    if not os.path.exists(directory_path):
        return

    for filename in os.listdir(directory_path):
        file_path = os.path.join(directory_path, filename)
        if os.path.isfile(file_path) or os.path.islink(file_path):
            os.unlink(file_path)
        elif os.path.isdir(file_path):
            shutil.rmtree(file_path)

    shutil.rmtree(directory_path)


def _bin_directory_name() -> str:
    return 'Bin' if os.name == 'nt' else 'bin'


def _executable_suffix() -> str:
    return '.exe' if os.name == 'nt' else ''


def resolve_vulkan_tool(vulkan_sdk_path: str, tool_name: str) -> str:
    if not vulkan_sdk_path:
        raise ValueError('vulkan_sdk_path must be provided')

    tool_path = Path(vulkan_sdk_path) / _bin_directory_name() / f"{tool_name}{_executable_suffix()}"
    return str(tool_path)


def get_dxc_path(vulkan_sdk_path: str) -> str:
    return resolve_vulkan_tool(vulkan_sdk_path, 'dxc')


def get_spirv_cross_path(vulkan_sdk_path: str) -> str:
    return resolve_vulkan_tool(vulkan_sdk_path, 'spirv-cross')


def get_glslc_path(vulkan_sdk_path: str) -> str:
    return resolve_vulkan_tool(vulkan_sdk_path, 'glslc')

import os
import shutil


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
    pass


def get_os_vulkan_path() -> str:
    return os.environ.get('VK_SDK_PATH')


def get_dxc_path() -> str:
    return get_os_vulkan_path() + '/Bin/dxc.exe'


def get_spirv_cross_path() -> str:
    return get_os_vulkan_path() + '/Bin/spirv-cross.exe'


def get_glslc_path() -> str:
    return get_os_vulkan_path() + '/Bin/glslc.exe'

import os.path
from tools.shader_compile import glslc
from tools.miscs import path_miscs


def main() -> None:
    src_dir: str = '../Assets/Shader/'

    path_miscs.clear_dir('../Assets/ShaderBin/')

    for root, dir_list, file_list in os.walk(src_dir):
        for file_name in file_list:
            file_full_path: str = root + '/' + file_name
            file_suffix: str = file_name.split('.')[-1]

            # compile glsl
            if file_suffix == 'vert' or file_suffix == 'frag':
                glslc.compile_glsl(file_full_path)

    pass


if __name__ == '__main__':
    main()
    pass

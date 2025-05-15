import subprocess
from ..miscs import path_miscs


def spv_decompile_glsl(spv_path: str) -> None:
    file_suffix: str = spv_path.split('.')[-1]
    if file_suffix != 'spv':
        return

    print('\tdecompile spv {}'.format(spv_path))
    glsl_src_path: str = spv_path.replace('.spv', '.spv.glsl')

    cmd = '{} {} --output {} --no-es'.format(path_miscs.get_spirv_cross_path(), spv_path, glsl_src_path)

    print('\tcall: {}'.format(cmd))
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if len(result.stdout) > 0:
        print('\t\t[spirv-cross] {}'.format(result.stdout))

    if len(result.stderr) > 0:
        print('\t\t[spirv-cross] {}'.format(result.stderr))

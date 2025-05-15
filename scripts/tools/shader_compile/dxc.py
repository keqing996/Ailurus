import os
import subprocess
from ..miscs import path_miscs


def _dxc_gen_cmd(hlsl_path: str, shader_type: str, out_path: str) -> str:
    cmd = path_miscs.get_dxc_path()
    if shader_type == 'vertex':
        cmd += ' -T vs_6_0 '
    elif shader_type == 'pixel':
        cmd += ' -T ps_6_0 '

    cmd += '-E main -Fo {}'.format(os.path.abspath(out_path))
    cmd += ' {}'.format(os.path.abspath(hlsl_path))
    return cmd


def hlsl_compile_dx_bin(hlsl_path: str, shader_type: str) -> None:
    file_suffix: str = hlsl_path.split('.')[-1]
    if file_suffix != 'hlsl':
        return

    print('compile hlsl {} src: {}\n'.format(shader_type, hlsl_path))
    bin_path: str = hlsl_path.replace('.hlsl', '.bin')

    cmd: str = _dxc_gen_cmd(hlsl_path, shader_type, bin_path)

    print('call: {}\n'.format(cmd))
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if len(result.stdout) > 0:
        print('[dxc] {}\n'.format(result.stdout))

    if len(result.stderr) > 0:
        print('[dxc] {}\n'.format(result.stderr))


def hlsl_compile_spirv_bin(hlsl_path: str, shader_type: str) -> None:
    file_suffix: str = hlsl_path.split('.')[-1]
    if file_suffix != 'hlsl':
        return

    print('compile hlsl {} src: {}\n'.format(shader_type, hlsl_path))
    bin_path: str = hlsl_path.replace('.hlsl', '.spv')

    cmd: str = _dxc_gen_cmd(hlsl_path, shader_type, bin_path)
    cmd += ' -spirv'

    print('call: {}\n'.format(cmd))
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if len(result.stdout) > 0:
        print('[dxc] {}\n'.format(result.stdout))

    if len(result.stderr) > 0:
        print('[dxc] {}\n'.format(result.stderr))

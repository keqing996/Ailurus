from tools.miscs import path_miscs
import shutil


def main() -> None:
    src_dir: str = '../Assets/'
    target_dir_debug: str = '../build_debug/Assets/'
    target_dir_release: str = '../build_release/Assets/'
    path_miscs.clear_dir(target_dir_debug)
    path_miscs.clear_dir(target_dir_release)
    shutil.copytree(src_dir, target_dir_debug)
    shutil.copytree(src_dir, target_dir_release)
    pass


if __name__ == '__main__':
    main()
    pass

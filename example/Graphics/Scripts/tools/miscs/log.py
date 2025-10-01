

def white(message: str) -> None:
    print(message)


def green(message: str) -> None:
    print(f"\033[32m{message}\033[0m")


def red(message: str) -> None:
    print(f"\033[31m{message}\033[0m")


def yellow(message: str) -> None:
    print(f"\033[33m{message}\033[0m")
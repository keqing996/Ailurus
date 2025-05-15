

class FunctionConfig:
    def __init__(self, name: str, return_type: str | None, input_para: dict[str, str], class_name: str) -> None:
        self.name: str = name
        self.return_type: str | None = return_type
        self.input_para: dict[str, str] = input_para
        self.class_name: str = class_name
        self.has_const: bool = False
        self.has_static: bool = False
        pass

    def add_const(self) -> None:
        self.has_const = True

    def add_static(self) -> None:
        self.has_static = True

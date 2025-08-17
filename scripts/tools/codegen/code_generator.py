from __future__ import annotations
from .enum_gen_config import EnumConfig
from .function_gen_config import FunctionConfig


class CodeGenerator:
    content: str
    indent: int

    def __init__(self) -> None:
        self.content = ''
        self.indent = 0

    def write_to_file(self, path: str) -> CodeGenerator:
        f = open(path, 'w', encoding='utf-8')
        f.write(self.content)
        f.close()
        return self

    def append(self, s: str) -> CodeGenerator:
        self.content += s
        return self

    def new_line(self) -> CodeGenerator:
        self.content += '\n'
        for i in range(self.indent):
            self.content += '\t'
        return self

    def left_bracket(self) -> CodeGenerator:
        self.content += '{'
        return self

    def right_bracket(self) -> CodeGenerator:
        self.content += '}'
        return self

    def semicolon(self) -> CodeGenerator:
        self.content += ';'
        return self

    def indent_increase(self) -> CodeGenerator:
        self.indent += 1
        return self

    def indent_decrease(self) -> CodeGenerator:
        self.indent -= 1
        return self

    def gen_header_comment(self) -> CodeGenerator:
        self.append('/* ****************************** \n')
        self.append(' * Auto Generated File ! \n')
        self.append(' * ******************************/\n')
        return self

    def gen_include(self, include_name: str) -> CodeGenerator:
        self.append('#include ' + include_name)
        return self

    def gen_pragma_once(self) -> CodeGenerator:
        self.append('#pragma once')
        return self

    def gen_namespace(self, namespace_name: str) -> CodeGenerator:
        self.append('namespace ' + namespace_name)
        return self

    def gen_function(self, function_config: FunctionConfig, is_declaration: bool) -> CodeGenerator:
        if not is_declaration:
            full_function_name = function_config.class_name + '::' + function_config.name
        else:
            full_function_name = function_config.name

        if is_declaration and function_config.has_static:
            self.append('static ')

        if function_config.return_type is None:
            self.append(full_function_name + '(')
        else:
            self.append(function_config.return_type + ' ' + full_function_name + '(')

        size: int = len(function_config.input_para)
        index: int = 0
        for para_type, para_name in function_config.input_para.items():
            self.append(para_type + ' ' + para_name)
            index += 1
            if index < size:
                self.append(', ')

        self.append(')')

        if function_config.has_const:
            self.append(' const')

        return self

    def gen_enum_class(self, enum_config: EnumConfig) -> CodeGenerator:
        self.append('enum class ' + enum_config.enum_name + ': ' + enum_config.enum_type).new_line()
        self.left_bracket().indent_increase().new_line()

        size: int = len(enum_config.value_array)
        index: int = 0
        for enum_value in enum_config.value_array:
            self.append(enum_value)
            index += 1
            if index < size:
                self.append(',')
                self.new_line()

        self.indent_decrease().new_line()
        self.right_bracket().semicolon()
        return self


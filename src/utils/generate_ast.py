import re

def define_type(base_name: str, class_name: str, fields_list: str):
    code = fr"""template<typename T>
struct {class_name} : public {base_name}<T> {{
    {class_name}({fields_list}) :
"""
    fields = re.split(", ", fields_list)
    for i, field in enumerate(fields):
        name = field.split(" ")[-1]
        code += f"\tm_{name.capitalize()}({name})"
        if i != len(fields) - 1:
            code += ","
        code += "\n"
    code += "\t{ }\n"

    code += fr"""
    inline T Accept(Visitor<T>* visitor) override {{
        return visitor->Visit{class_name}{base_name}(this);          
    }}

"""

    for i, field in enumerate(fields):
        type_ = field.split(" ")[-2]
        if type_[len(type_) - 1] == "&":
            type_ = type_[:-1]
        name = field.split(" ")[-1]
        code += f"\t{type_} m_{name.capitalize()};\n"
    code += "};\n\n"
    return code

def define_visitor(base_name: str, types: list):
    code = r"""template<typename T>
class Visitor {
public:
    virtual ~Visitor() = 0;
"""
    for type_ in types:
        class_name = type_.split(";")[0].strip()
        code += f"\tvirtual T Visit{class_name}{base_name}({class_name}<T>* {base_name.lower()}) = 0;\n"
    code += "};\n\ntemplate<typename T>\ninline Visitor<T>::~Visitor() = default;"
    return code
        

def define_ast(base_name: str, types: list):
    source_code = r"""#pragma once
#include <variant>
#include "Token.h"

template<typename T>
class Visitor;

template<typename T>
struct Expr {
    virtual ~Expr() = 0;

    virtual T Accept(Visitor<T>* visitor) = 0;
};

template<typename T>
inline Expr<T>::~Expr() = default;

"""

    for type_ in types:
        class_name = type_.split(";")[0].strip()
        fields = type_.split(";")[1].strip()
        source_code += define_type(base_name, class_name, fields)

    source_code += define_visitor(base_name, types)

    with open(f"{base_name}.h", "w") as f:
        f.write(source_code)


if __name__ == "__main__":
    # class_name ; fields
    types = [
        "Binary   ; Expr<T>* left, const Token& opr, Expr<T>* right",
        "Grouping ; Expr<T>* expression",
        "Literal  ; const std::variant<std::monostate,double,std::string>& value",
        "Unary    ; const Token& opr, Expr<T>* right"
    ]

    define_ast("Expr", types)

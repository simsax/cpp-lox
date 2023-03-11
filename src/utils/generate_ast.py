import re


def define_type(base_name: str, class_name: str):
    return fr"""
inline std::any {class_name}::Accept(const Visitor& visitor) {{
	return visitor.Visit{class_name}(this);
}}
"""


def declare_type(base_name: str, class_name: str, fields_list: str):
    code = fr"""struct {class_name} : public {base_name} {{
    {class_name}({fields_list}) :
"""
    fields = re.split(", ", fields_list)
    for i, field in enumerate(fields):
        name = field.split(" ")[-1]
        if name == "opr" or name == "value":
            code += f"\tm_{name.capitalize()}({name})"
        else:
            code += f"\tm_{name.capitalize()}(std::move({name}))"
        if i != len(fields) - 1:
            code += ","
        code += "\n"
    code += "\t{ }\n"

    code += r"""
    std::any Accept(const Visitor& visitor) override;

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
    code = r"""class Visitor {
public:
    virtual ~Visitor() = 0;
"""
    for type_ in types:
        class_name = type_.split(";")[0].strip()
        code += f"\tvirtual std::any Visit{class_name}({class_name}* {base_name.lower()}) const = 0;\n"
    code += "};\n\ninline Visitor::~Visitor() = default;\n"
    return code


def define_ast(base_name: str, types: list):
    source_code = fr"""#pragma once
#include <any>
#include <memory>
#include "Token.h"

namespace {base_name.lower()} {{

class Visitor;

struct {base_name} {{
    virtual ~{base_name}() = 0;

    virtual std::any Accept(const Visitor& visitor) = 0;
}};

inline {base_name}::~{base_name}() = default;

"""

    for type_ in types:
        class_name = type_.split(";")[0].strip()
        fields = type_.split(";")[1].strip()
        source_code += declare_type(base_name, class_name, fields)

    source_code += define_visitor(base_name, types)

    for type_ in types:
        class_name = type_.split(";")[0].strip()
        source_code += define_type(base_name, class_name)

    source_code += "\n}\n" # close namespace

    with open(f"{base_name}.h", "w") as f:
        f.write(source_code)


if __name__ == "__main__":
    # class_name ; fields
    exprTypes = [
        "Binary   ; std::unique_ptr<Expr> left, const Token& opr, std::unique_ptr<Expr> right",
        "Grouping ; std::unique_ptr<Expr> expression",
        "Literal  ; const std::any& value",
        "Unary    ; const Token& opr, std::unique_ptr<Expr> right"
    ]

    stmtTypes = [
        "Expression ; std::unique_ptr<expr::Expr> expression",
        "Print      ; std::unique_ptr<expr::Expr> expression"
    ]

    define_ast("Expr", exprTypes)
    define_ast("Stmt", stmtTypes)

# Import and env is provided by platformio
Import("env")  # pyright: ignore[reportUndefinedVariable]

build_tool_deps = ["fontTools[woff]"]

for d in build_tool_deps:
    env.Execute(  # pyright: ignore[reportUndefinedVariable]
        f"$PYTHONEXE -m pip install {d}"
    )  # pyright: ignore[reportUndefinedVariable]

from pathlib import Path

# Import and env is provided by platformio
Import("env")  # pyright: ignore[reportUndefinedVariable]

project_dir = Path(
    env.subst("$PROJECT_DIR") # pyright: ignore[reportUndefinedVariable]
)

assets_dir = project_dir / "web"
assets_target_dir = project_dir / "include" / "web_remote" / "assets"
assets_target_dir.mkdir(parents=True, exist_ok=True)

for path in assets_dir.rglob("*"):
    if path.is_file():
        target_file = assets_target_dir / path.relative_to(assets_dir)
        asset_name = (
            str(path.relative_to(assets_dir))
            .replace("/", "_")
            .replace(".", "_")
            .replace("-", "_")
            .upper()
        )
        target_file.parent.mkdir(parents=True, exist_ok=True)

        with target_file.with_name(path.name + ".h").open("w") as f:
            f.writelines(
                [
                    "#pragma once\n\n",
                    "#include <Arduino.h>\n\n",
                    "namespace assets {\n\n"
                    f'const char {asset_name}[] PROGMEM = R"rawliteral(\n',
                    path.read_text(),
                    '\n)rawliteral";\n\n}\n',
                ]
            )

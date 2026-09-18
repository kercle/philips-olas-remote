from pathlib import Path
from fontTools import subset

REGULAR_FONT_GLYPHS = "U+f185,U+f0eb"
SOLID_FONT_GLYPHS = "U+f185,U+f0eb,U+f863,U+f2f1,U+f6c3"


def subset_font(input_font: str, output_font: str, glyphs: str) -> None:
    options = subset.Options()

    options.flavor = "woff2"
    options.layout_features = ["*"]

    font = subset.load_font(input_font, options)
    sub_setter = subset.Subsetter(options=options)

    sub_setter.populate(unicodes=subset.parse_unicodes(glyphs))
    sub_setter.subset(font)

    subset.save_font(font, output_font, options)


# Import and env is provided by platformio
Import("env")  # pyright: ignore[reportUndefinedVariable]

project_dir = Path(
    env.subst("$PROJECT_DIR")  # pyright: ignore[reportUndefinedVariable]
)

web_assets_dir = project_dir / "web"

subset_font(
    web_assets_dir / "webfonts" / "fa-regular-400.woff2",
    web_assets_dir / "webfonts" / "fa-regular-custom.woff2",
    REGULAR_FONT_GLYPHS,
)

subset_font(
    web_assets_dir / "webfonts" / "fa-solid-900.woff2",
    web_assets_dir / "webfonts" / "fa-solid-custom.woff2",
    SOLID_FONT_GLYPHS,
)

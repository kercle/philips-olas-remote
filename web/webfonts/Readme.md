# Subsetting embedded webfonts

Since we don't need the full set of Fontawesome glyphs, we can generate a customized subset using:

```bash
pyftsubset fa-solid-900.woff2 \
  --unicodes="U+f185,U+f0eb,U+f863,U+f2f1,U+f6c3" \
  --flavor=woff2 \
  --layout-features="*" \
  --output-file=fa-solid-custom.woff2

pyftsubset fa-regular-400.woff2 \
  --unicodes="U+f185,U+f0eb" \
  --flavor=woff2 \
  --layout-features="*" \
  --output-file=fa-regular-custom.woff2
```

> [!WARNING]  
> Sub-setting also happens as part of the pre-scripts when compiling PlatformIO targets. For adding a glyph, modify the script `scripts/subset_webfonts.py`.

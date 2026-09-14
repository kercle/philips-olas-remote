# Subsetting embedded webfonts

Since we don't need the full set of Fontawesome glyphs, we can generate a customized subset using:

```bash
pyftsubset fa-solid-900.woff2 \
  --unicodes="U+F185,U+F863" \
  --flavor=woff2 \
  --layout-features="*" \
  --output-file=fa-solid-custom.woff2

pyftsubset fa-regular-400.woff2 \
  --unicodes="U+F185" \
  --flavor=woff2 \
  --layout-features="*" \
  --output-file=fa-regular-custom.woff2

```

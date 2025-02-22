# Requires pandoc and LaTeX.

import os
import pypandoc
import re

output_path = "./manual.pdf"

# Concatenate Markdown files in the given order
input_files = [
    "index.md", "symbos.md", "syscall1.md", "syscall2.md",
    "graphics.md", "network.md", "special.md", "porting.md"
]
combined_md = "combined.md"

# Read and concatenate Markdown files, fixing internal links
with open(combined_md, "w") as outfile:
    for fname in input_files:
        with open(fname, "r") as infile:
            content = infile.read()
            # Convert [link](file.md#anchor) -> [link](#anchor)
            content = re.sub(r'\[([^\]]+)\]\([^\)]+\.md(#.*?)\)', r'[\1](\2)', content)
            outfile.write(content + "\n\n")

pandoc_cmd = [
    "--include-in-header", "pandoc.tex",  # Use external LaTeX header settings
    "-V", "linkcolor:blue",
    "-V", "geometry:letterpaper",
    "-V", "geometry:left=1.8cm,right=1.8cm,top=2.75cm,bottom=2.25cm",
    "-V", "mainfont=Cambria",
    "-V", "fontsize=12pt",
    "-V", "monofont=SauceCodePro Nerd Font",
    "--metadata", "title=SymbOS C Compiler Manual",
    "--metadata", "author=v1.2",
    "--toc",
    "--toc-depth=3",
    "--pdf-engine=xelatex",    # Required for title page
]

pypandoc.convert_file(
    combined_md,  # Use concatenated file to preserve order
    "pdf",
    outputfile=output_path,
    extra_args=pandoc_cmd,
)

os.remove(combined_md)

print("Output saved to", output_path)
